#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

uniform mat4 u_MVP;

out vec4 v_Color;

void main()
{
	gl_Position = u_MVP * vec4(position, 1.0);
	v_Color = color;
}

#shader fragment
#version 330 core

in vec4 v_Color;
out vec4 FragColor;

void main()
{
	FragColor = v_Color;
}