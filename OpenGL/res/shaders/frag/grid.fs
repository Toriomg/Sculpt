#version 330 core

in vec3 v_worldPos;
uniform vec3 u_cameraPos;
out vec4 FragColor;

float grid_intensity(vec2 coord, float width) {
    vec2 fw = fwidth(coord);
    vec2 grid_uv = fract(coord);
    vec2 line = smoothstep(fw * width, vec2(0.0), grid_uv) - smoothstep(vec2(1.0) - fw * width, vec2(1.0), grid_uv);
    return max(line.x, line.y);
}

void main()
{
    vec3 direction = normalize(v_worldPos - u_cameraPos);
    float t = -u_cameraPos.y / direction.y;

    if (t < 0.0) {
        discard;
    }

    vec3 groundPos = u_cameraPos + direction * t;
    float dist = length(groundPos - u_cameraPos);
    
    // Calculate visibility for the finer grids based on their screen-space size
    float tiny_vis = smoothstep(0.3, 0.1, max(fwidth(groundPos.xz).x, fwidth(groundPos.xz).y));
    float mid_vis = smoothstep(1.25, 0.7, max(fwidth(groundPos.xz / 10.0).x, fwidth(groundPos.xz / 10.0).y));

    // Calculate the intensity of each grid level and multiply by its visibility
    // If visibility is 0, the intensity becomes 0.
    float tiny_intensity = grid_intensity(groundPos.xz, 1.0) * tiny_vis;
    float mid_intensity = grid_intensity(groundPos.xz / 10.0, 1.75) * mid_vis;
    float major_intensity = grid_intensity(groundPos.xz / 100.0, 5.5); // Major grid is always visible

    // Combine the intensities. Use max() so lines don't get "brighter" at intersections.
    float total_intensity = max(major_intensity, max(mid_intensity, tiny_intensity));

    vec3 lineColor = vec3(0.0);
    vec4 gridSourceColor = vec4(lineColor, total_intensity);

    float fogFactor = 1.0 - exp(-dist * 0.005);
    vec4 fogColor = vec4(0.2f, 0.3f, 0.3f, 1.0f); // Background color

    gridSourceColor.a *= 1.0 - fogFactor;
    FragColor = gridSourceColor;
}