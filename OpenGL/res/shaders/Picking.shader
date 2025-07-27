#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

#shader fragment
#version 330 core

layout (location = 0) out uvec4 FragColor;

uniform uint objectID;

void main()
{
    FragColor = uvec4(objectID, 0, 0, 1); 
}