#shader vertex
#version 330 core

uniform mat4 u_MVP;

const vec3 Pos[4] = vec3[4](
	vec3(-0.5, -0.5, 0.0), // Bottom Left
	vec3( 0.5, -0.5, 0.0), // Bottom Right
	vec3( 0.5,  0.5, 0.0), // Top Right
	vec3(-0.5,  0.5, 0.0)  // Top Left
);

const int Indices[6] = int[6](
	0, 1, 2, // First Triangle
	2, 0, 3  // Second Triangle
);

void main()
{
	int Index = Indices[gl_VertexID];
	vec4 vPos = vec4(Pos[Index], 1.0);
	gl_Position = u_MVP * vPos;
}

#shader fragment
#version 330 core
layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(0.0); // Red color
}