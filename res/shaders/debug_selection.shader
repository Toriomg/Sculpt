#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
uniform mat4 u_ViewProjection;
uniform mat4 u_Model;
void main() {
    gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}

#shader fragment
#version 330 core
out vec4 FragColor;
uniform int u_IsPickable;
void main() {
    FragColor = (u_IsPickable == 1)
        ? vec4(0.2, 0.9, 0.2, 1.0)
        : vec4(0.9, 0.2, 0.2, 1.0);
}
