#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position; // The vertex position (from your VBO, layout.Push<float>(3))

uniform mat4 u_ViewProjection; // The combined view and projection matrix
uniform mat4 u_Model;          // The model matrix for the object


void main()
{
    gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}


// --- Fragment Shader ---
#shader fragment
#version 330 core

out vec4 color; // The output color of this fragment

uniform vec4 u_Color; // A uniform color that we set from the C++ application
void main()
{
color = u_Color;
}