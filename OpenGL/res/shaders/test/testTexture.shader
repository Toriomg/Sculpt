// modelmesh.shader - Fragment Shader

#version 330 core
out vec4 FragColor;

in vec2 v_TexCoord;
uniform sampler2D u_Texture;

void main()
{
    // Ignoramos la textura y los UVs por completo.
    // Si el objeto se está dibujando, TIENE que ser rojo.
    FragColor = vec4(1.0, 0.0, 1.0, 1.0); // Rojo brillante (o magenta, para que sea obvio)
}