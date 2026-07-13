#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
    gl_PointSize = 5.0;
}

#shader fragment
#version 330 core
out vec4 FragColor;

uniform vec4 u_Color;

void main()
{
    FragColor = u_Color;
}
