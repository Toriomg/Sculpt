#version 330 core

uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_worldPos;

// A quad that fills the entire screen in clip space
const vec3 vertices[4] = vec3[4](
	vec3(-1.0, -1.0, 1.0),
	vec3( 1.0, -1.0, 1.0),
	vec3(-1.0,  1.0, 1.0),
	vec3( 1.0,  1.0, 1.0)
);

const int indices[6] = int[6](
	0, 1, 2, 1, 3, 2
);

void main()
{
    // 1. Unproject the screen-space vertex to find its position on the far plane.
    vec4 pos = inverse(u_projection) * vec4(vertices[indices[gl_VertexID]].xy, 1.0, 1.0);
    // 2. Transform that position from camera space to world space.
    pos = inverse(u_view) * pos;
    // 3. Pass the calculated world position to the fragment shader.
    v_worldPos = pos.xyz / pos.w;

    // We output the clip-space position directly to create a full-screen quad
    gl_Position = vec4(vertices[indices[gl_VertexID]].xy, 1.0, 1.0);
}