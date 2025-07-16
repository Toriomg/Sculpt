#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 u_MVP;
uniform sampler2D u_Textures[2];

out vec4 v_Color;
out vec2 v_TexCoord;
out vec3 v_Normal;


void main()
{
	gl_Position = u_MVP * vec4(position, 1.0);
	v_TexCoord = texCoord;
	v_Normal = normal;
}

#shader fragment
#version 330 core

in vec4 v_Color;
in vec2 v_TexCoord;
in vec3 v_Normal;
out vec4 FragColor;

uniform sampler2D u_Textures[2];

void main()
{
	FragColor = texture(u_Textures[0], v_TexCoord);
}