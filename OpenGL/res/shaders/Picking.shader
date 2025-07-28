#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 u_MVP;
uniform mat4 u_Model;

out vec3 v_WorldPos;

void main()
{
    v_WorldPos = vec3(u_Model * vec4(aPos, 1.0));
    gl_Position = u_MVP * vec4(aPos, 1.0);
}

#shader fragment
#version 330 core

layout (location = 0) out uvec3 out_IDs;
layout (location = 1) out vec4 out_WorldPos;

uniform uint objectID;

in vec3 v_WorldPos;

void main()
{
    out_IDs  = uvec3(objectID, gl_PrimitiveID, 0);
    out_WorldPos = vec4(v_WorldPos, 1.0);
}