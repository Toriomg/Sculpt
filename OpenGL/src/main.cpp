#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h" // Include the Renderer header for GLCall macro

#include "buffers/VertexBuffer.h"
#include "buffers/VertexBufferLayout.h"
#include "buffers/IndexBuffer.h"
#include "buffers/VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp" // Include GLM for vector and matrix operations
#include "glm/gtc/matrix_transform.hpp" // Include GLM for matrix transformations

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

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
    window = glfwCreateWindow(960, 540, "Hello World", NULL, NULL);
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
             100.0f,  100.0f, 0.0f, 0.0f, // Bottom left
             200.0f,  100.0f, 1.0f, 0.0f, // Bottom right
             200.0f,  200.0f, 1.0f, 1.0f,  // Top right
             100.0f,  200.0f, 0.0f, 1.0f // Top left
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

		GLCall(glEnable(GL_BLEND)); // Enable blending for transparency
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // Set the blending function for transparency

		VertexArray va; // Create a Vertex Array Object (VAO) to hold the vertex attributes
        VertexBuffer vb(positions, sizeof(positions)); // Create a Vertex Buffer Object (VBO) with the vertex data

        VertexBufferLayout layout;
		layout.Push<float>(2); 
        layout.Push<float>(2);
		va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, sizeof(indices)); // Create an Index Buffer Object (IBO) with the index data

        glm::mat4 proj = glm::ortho(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-100.0f, 0.0f, 0.0f));
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 200.0f, 0.0f));

        glm::mat4 mvp = proj * view * model;

        glm::vec4 vp(100.0f, 100.0f, 0.0f, 1.0f);

        
        // Parse the shader file
		Shader shader("res/shaders/Basic.shader"); // Create a Shader object with the shader file path
		shader.Bind(); // Bind the shader program
        shader.SetUniform4f("u_Color", 0.2f, 0.3f, 0.8f, 1.0f);
        shader.SetUniformMat4f("u_MVP", mvp);

        Texture texture("res/textures/texture1.png");
		texture.Bind(); // Bind the texture 
		shader.SetUniform1i("u_Texture", 0); // Set the texture uniform in the shader

		va.Unbind(); // Unbind the VAO
		shader.Unbind(); // Unbind the shader program
		vb.Unbind(); // Unbind the VBO
		ib.Unbind(); // Unbind the IBO

		Renderer renderer; // Create a Renderer object to handle OpenGL calls

		// Setting up ImGui
		IMGUI_CHECKVERSION(); // Check ImGui version
		ImGui::CreateContext(); // Initialize ImGui context
		ImGuiIO& io = ImGui::GetIO(); // Get ImGui IO object
		(void)io; // Suppress unused variable warning
		ImGui::StyleColorsDark(); // Set ImGui style to dark
		ImGui_ImplGlfw_InitForOpenGL(window, true); // Initialize ImGui for GLFW
        ImGui_ImplOpenGL3_Init("#version 330"); // Initialize ImGui for OpenGL 3.3


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

			ImGui_ImplOpenGL3_NewFrame(); // Start a new ImGui frame
            ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame(); // Create a new ImGui frame


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

            ImGui::Begin("My name is window, ImGui window");
			ImGui::Text("Hello, world!"); // Display text in the ImGui window
			ImGui::End(); // End the ImGui window

			ImGui::Render(); // Render ImGui
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Render ImGui draw data

            /* Swap front and back buffers */
            glfwSwapBuffers(window);
            /* Poll for and process events */
            glfwPollEvents();
        }
    }

	ImGui_ImplOpenGL3_Shutdown(); // Shutdown ImGui for OpenGLç
    ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(); // Destroy ImGui context

    glfwTerminate();
    return 0;
}