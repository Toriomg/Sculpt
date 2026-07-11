#shader vertex
#version 330 core

// This shader uses gl_VertexID to generate a triangle's vertices
// directly in the shader, without needing any input from a VBO.
// This is a powerful way to test if your rendering pipeline is working
// at the most basic level.

// An array of the 3 vertex positions for our triangle in screen space [-1, 1].
const vec2 positions[3] = vec2[](
    vec2(-0.5, -0.5), // Bottom-left
    vec2( 0.5, -0.5), // Bottom-right
    vec2( 0.0,  0.5)  // Top-center
);

void main()
{
    // gl_VertexID is a built-in variable that contains the index of the
    // vertex currently being processed. When we call glDrawArrays(GL_TRIANGLES, 0, 3),
    // this shader will run 3 times with gl_VertexID being 0, 1, and 2.
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
}


#shader fragment
#version 330 core

// The final output color for each pixel.
out vec4 FragColor;

void main()
{
    // Output a solid, bright, unmissable color.
    // Magenta is great for debugging.
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}