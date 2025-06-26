#include "TestBatchRendering.h"
#include "../Renderer.h"

#include "imgui/imgui.h"

#include "glm/glm.hpp" // Include GLM for vector and matrix operations
#include "glm/gtc/matrix_transform.hpp" // Include GLM for matrix transformations

#include <GL/glew.h>

const float WINDW_SIZE_X = 960.0f; // Define the window width
const float WINDW_SIZE_Y = 540.0f; // Define the window height

namespace test {
	TestBatchRendering::TestBatchRendering()
		:m_Translation(200.0f, 200.0f, 0.0f),
		m_Proj(glm::ortho(0.0f, WINDW_SIZE_X, 0.0f, WINDW_SIZE_Y, -1.0f, 1.0f)),
		m_View(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)))
	{
		float positions[] = {
			-150.0f, -50.0f, 0.0f, 0.0f, // Bottom left
			-50.0f, -50.0f, 1.0f, 0.0f, // Bottom right
			-50.0f,  50.0f, 1.0f, 1.0f,  // Top right
			-150.0f,  50.0f, 0.0f, 1.0f, // Top left

			 50.0f, -50.0f, 0.0f, 0.0f, // Bottom left
			 150.0f, -50.0f, 1.0f, 0.0f, // Bottom right
			 150.0f,  50.0f, 1.0f, 1.0f,  // Top right
			 50.0f,  50.0f, 0.0f, 1.0f // Top left
		};

		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0,

			4 ,5, 6,
			6, 7, 4
		};

		m_VAO = std::make_unique<VertexArray>(); // Create a Vertex Array Object (VAO) to hold the vertex attributes
		m_VBO = std::make_unique<VertexBuffer>(positions, sizeof(positions)); // Create a Vertex Buffer Object (VBO) with the vertex data
		VertexBufferLayout layout;
		layout.Push<float>(2);
		layout.Push<float>(2);

		m_VAO->AddBuffer(*m_VBO, layout);
		m_IBO = std::make_unique<IndexBuffer>(indices, sizeof(indices)); // Create an Index Buffer Object (IBO) with the index data

		// Parse the shader file
		m_Shader = std::make_unique<Shader>("res/shaders/BasicColor.shader"); // Create a Shader object with the shader file path	
		m_Shader->Bind(); // Bind the shader program

		m_Shader->SetUniform4f("u_Color", 1.0f, 0.0f, 0.0f, 1.0f);
	}

	TestBatchRendering::~TestBatchRendering() {

	}

	void TestBatchRendering::OnUpdate(float deltaTime) {

	}
	void TestBatchRendering::OnRender() {
		GLCall(glClearColor(0.2f, 0.3f, 0.3f, 1.0f)); // Set the clear color
		GLCall(glClear(GL_COLOR_BUFFER_BIT)); // Clear the color buffer

		Renderer renderer; // Create a Renderer object to handle drawing

		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), m_Translation);
			glm::mat4 mvp = m_Proj * m_View * model;
			m_Shader->Bind();
			m_Shader->SetUniformMat4f("u_MVP", mvp);
			// Draw the object using the Renderer
			renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
		}
	}
	void TestBatchRendering::OnImGuiRender() {
		ImGui::Text("Batch Rendering"); // Display text in the ImGui window
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
}