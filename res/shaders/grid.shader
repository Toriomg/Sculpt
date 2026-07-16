#shader vertex
#version 330 core

uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_worldPos;

// Full-screen quad defined in clip space; vertices are generated from gl_VertexID so no VBO is needed.
const vec3 vertices[4] = vec3[4](
    vec3(-1.0, -1.0, 1.0),
    vec3( 1.0, -1.0, 1.0),
    vec3(-1.0,  1.0, 1.0),
    vec3( 1.0,  1.0, 1.0)
);

const int indices[6] = int[6](0, 1, 2, 1, 3, 2);

void main()
{
    // Unproject the clip-space vertex onto the far plane in world space so the fragment
    // shader can ray-cast from the camera position through each pixel to Y=0.
    vec4 pos = inverse(u_projection) * vec4(vertices[indices[gl_VertexID]].xy, 1.0, 1.0);
    pos = inverse(u_view) * pos;
    v_worldPos = pos.xyz / pos.w;

    // z=1/w=1 places every vertex on the far plane in NDC; drawn with GL_LEQUAL so it
    // renders only where no opaque geometry has written a closer depth value.
    gl_Position = vec4(vertices[indices[gl_VertexID]].xy, 1.0, 1.0);
}

#shader fragment
#version 330 core

in vec3 v_worldPos;
uniform vec3 u_cameraPos;
uniform vec3 u_gridColor;
out vec4 FragColor;

// Returns the intensity of a grid line at 'coord' with a line half-width of 'width' pixels.
// Uses fwidth for anti-aliasing that scales correctly at any zoom level.
float grid_intensity(vec2 coord, float width) {
    vec2 fw = fwidth(coord);
    vec2 grid_uv = fract(coord);
    vec2 line = smoothstep(fw * width, vec2(0.0), grid_uv)
              - smoothstep(vec2(1.0) - fw * width, vec2(1.0), grid_uv);
    return max(line.x, line.y);
}

void main()
{
    vec3 direction = normalize(v_worldPos - u_cameraPos);

    // Ray-plane intersection: t = distance along the ray to the Y=0 ground plane.
    // t < 0 means the ground plane is behind the camera for this pixel.
    float t = -u_cameraPos.y / direction.y;
    if (t < 0.0) {
        discard;
    }

    vec3 groundPos = u_cameraPos + direction * t;
    float dist = length(groundPos - u_cameraPos);

    // Each finer grid scale fades out when its line spacing shrinks below ~1 screen pixel,
    // preventing aliasing and visual noise close to the horizon.
    float tiny_vis = smoothstep(0.3, 0.1, max(fwidth(groundPos.xz).x,        fwidth(groundPos.xz).y));
    float mid_vis  = smoothstep(1.0, 0.5, max(fwidth(groundPos.xz / 10.0).x, fwidth(groundPos.xz / 10.0).y));

    float tiny_intensity  = grid_intensity(groundPos.xz,         1.0)  * tiny_vis;
    float mid_intensity   = grid_intensity(groundPos.xz / 10.0,  1.75) * mid_vis;
    float major_intensity = grid_intensity(groundPos.xz / 100.0, 5.5); // always visible

    float total_intensity = max(major_intensity, max(mid_intensity, tiny_intensity));

    vec4 gridColor = vec4(u_gridColor, total_intensity);

    // Exponential fog fades the grid out at distance so it doesn't dominate the horizon.
    float fogFactor = 1.0 - exp(-dist * 0.005);
    gridColor.a *= 1.0 - fogFactor;

    FragColor = gridColor;
}
