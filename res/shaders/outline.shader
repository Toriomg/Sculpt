#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;
uniform float u_OutlineThickness;

void main()
{
    gl_Position = u_ViewProjection * u_Model * vec4(position, 1.0);
    vec2 clipNormal = (u_ViewProjection * u_Model * vec4(normal, 0.0)).xy;
    float len = length(clipNormal);
    if (len > 0.001)
        gl_Position.xy += (clipNormal / len) * u_OutlineThickness * gl_Position.w;
}

#shader fragment
#version 330 core
out vec4 FragColor;
uniform vec4 u_OutlineColor;
void main()
{
    FragColor = u_OutlineColor;
}
