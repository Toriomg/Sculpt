#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position; // The vertex position (from your VBO, layout.Push<float>(3))

uniform mat4 u_ViewProjection; // The combined view and projection matrix
uniform mat4 u_Model;          // The model matrix for the object

out vec3 v_Color; // Pass color to fragment shader

void main()
{
    int id = gl_VertexID % 6; // choose 6 different colors
    if (id == 0) v_Color = vec3(1.0, 0.0, 0.0);     // red
    else if (id == 1) v_Color = vec3(0.0, 1.0, 0.0); // green
    else if (id == 2) v_Color = vec3(0.0, 0.0, 1.0); // blue
    else if (id == 3) v_Color = vec3(1.0, 1.0, 0.0); // yellow
    else if (id == 4) v_Color = vec3(1.0, 0.0, 1.0); // magenta
    else              v_Color = vec3(0.0, 1.0, 1.0); // cyan

    // Combine transformations: model -> view -> projection
    // Order matters: first model, then view, then projection
    gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}


// --- Fragment Shader ---
#shader fragment
#version 330 core

in vec3 v_Color; 

out vec4 color; // The output color of this fragment

uniform vec4 u_Color; // A uniform color that we set from the C++ application

void main()
{
    color = vec4(v_Color, 1.0); // Simply output the uniform color for every pixel
}