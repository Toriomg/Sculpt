#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h" // Include the Renderer header for GLCall macro

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set the major version of OpenGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Set the minor version of OpenGL
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use the core profile of OpenGL

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

	glfwSwapInterval(1); // Enable vsync

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

	std::cout << glGetString(GL_VERSION) << std::endl;
    {

        float positions[] = {
            -0.5f, -0.5f, // Bottom left
             0.5f, -0.5f, // Bottom right
             0.5f,  0.5f,  // Top right
            -0.5f,  0.5f // Top left
        };

        unsigned int indices[6] = {
            0, 1, 2,
            2, 3, 0
        };

        unsigned int vaoID;
        GLCall(glGenVertexArrays(1, &vaoID)); // Generate a Vertex Array Object (VAO)
        GLCall(glBindVertexArray(vaoID)); // Bind the VAO
        // Create a Vertex Buffer Object (VBO) and upload vertex data

		VertexArray va; // Create a Vertex Array Object (VAO) to hold the vertex attributes
        VertexBuffer vb(positions, sizeof(positions)); // Create a Vertex Buffer Object (VBO) with the vertex data

        VertexBufferLayout layout;
		layout.Push<float>(2); 
		va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, sizeof(indices)); // Create an Index Buffer Object (IBO) with the index data

        // Parse the shader file
		Shader shader("res/shaders/Basic.shader"); // Create a Shader object with the shader file path
		shader.Bind(); // Bind the shader program
        shader.SetUniform4f("u_Color", 0.2f, 0.3f, 0.8f, 1.0f);

		va.Unbind(); // Unbind the VAO
		shader.Unbind(); // Unbind the shader program
		vb.Unbind(); // Unbind the VBO
		ib.Unbind(); // Unbind the IBO

		Renderer renderer; // Create a Renderer object to handle OpenGL calls

        float r = 0.2f;
        float g = 0.1f;
        float b = 0.2f;
        float a = 1.0f;
        float var = 0.01f; // Variable to control color change
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            glClear(GL_COLOR_BUFFER_BIT);

			shader.Bind(); // Bind the shader program
			shader.SetUniform4f("u_Color", r, g, b, a); // Set the uniform color variable in the shader

			renderer.Draw(va, ib, shader); // Draw the object using the Renderer

            if (r >= 1.0f) {
                var = -0.01f; // Reverse direction when reaching 1.0
            }
            else if (r <= 0.0f) {
                var = 0.01f; // Reverse direction when reaching 0.0
            }
            r += var; // Increment the red component

            /* Swap front and back buffers */
            glfwSwapBuffers(window);
            /* Poll for and process events */
            glfwPollEvents();
        }
    }
    glfwTerminate();
    return 0;
}