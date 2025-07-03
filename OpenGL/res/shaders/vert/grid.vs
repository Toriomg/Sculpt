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
    // Unproject the screen-space quad vertex to find its position on the far plane in world space
    vec4 pos = inverse(u_projection) * vec4(vertices[indices[gl_VertexID]].xy, 1.0, 1.0);
    pos = inverse(u_view) * pos;
    v_worldPos = pos.xyz / pos.w;

    // We output the clip-space position directly to create a full-screen quad
    gl_Position = vec4(vertices[indices[gl_VertexID]].xy, 1.0, 1.0);
}