#include "TestTexture2D.h"
#include "../Renderer.h"

#include "imgui/imgui.h"


#include <GL/glew.h>

const float WINDW_SIZE_X = 960.0f; // Define the window width
const float WINDW_SIZE_Y = 540.0f; // Define the window height

namespace test {
	TestTexture2D::TestTexture2D()
		:m_TranslationA(200.0f, 200.0f, 0.0f), m_TranslationB(400.0f, 200.0f, 0.0f),
		m_Proj(Matx4f::orthographic(0.0f, WINDW_SIZE_X, 0.0f, WINDW_SIZE_Y, -10000.0f, 1000000.0f)),
		m_View(Matx4f::translation(Vec3(0.0f, 0.0f, 0.0f)))
	{
		float positions[] = {
			-50.0f, -50.0f, 70.0f, 0.0f, 0.0f, // Bottom left
			 50.0f, -50.0f, 70.0f, 1.0f, 0.0f, // Bottom right
			 50.0f,  50.0f, 0.0f, 1.0f, 1.0f,  // Top right
			-50.0f,  50.0f, 0.0f, 0.0f, 1.0f // Top left
		};

		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		m_VAO = std::make_unique<VertexArray>(); // Create a Vertex Array Object (VAO) to hold the vertex attributes
		m_VBO = std::make_unique<VertexBuffer>(positions, sizeof(positions), true); // Create a Vertex Buffer Object (VBO) with the vertex data
		VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(2);

		m_VAO->AddBuffer(*m_VBO, layout);
		m_IBO = std::make_unique<IndexBuffer>(indices, sizeof(indices)); // Create an Index Buffer Object (IBO) with the index data

		// Parse the shader file
		m_Shader = std::make_unique<Shader>("res/shaders/BasicTexture.shader"); // Create a Shader object with the shader file path	
		m_Shader->Bind(); // Bind the shader program

		m_Texture = std::make_unique<Texture>("res/textures/texture1.png");
		m_Texture->Bind(); // Bind the texture 
		m_Shader->SetUniform1i("u_Texture", 0); // Set the texture uniform in the shader
	}

	TestTexture2D::~TestTexture2D() {

	}

	void TestTexture2D::OnUpdate(float deltaTime) {

	}
	void TestTexture2D::OnRender() {
		GLCall(glClearColor(0.2f, 0.3f, 0.3f, 1.0f)); // Set the clear color
		GLCall(glClear(GL_COLOR_BUFFER_BIT)); // Clear the color buffer

		Renderer renderer; // Create a Renderer object to handle drawing

		m_Texture->Bind(); // Bind the texture before drawing
		{
			Matx4f model = Matx4f::translation(m_TranslationA);
			Matx4f mvp = m_Proj * m_View * model;
			m_Shader->Bind();
			m_Shader->SetUniformMat4f("u_MVP", mvp);
			// Draw the object using the Renderer
			renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
		}

		{
			Matx4f model = Matx4f::translation(m_TranslationB);
			Matx4f mvp = m_Proj * m_View * model;
			m_Shader->Bind();
			m_Shader->SetUniformMat4f("u_MVP", mvp);
			// Draw the object using the Renderer
			renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
		}
	}
	void TestTexture2D::OnImGuiRender() {
		ImGui::Text("Texture 2D"); // Display text in the ImGui window
		ImGui::SliderFloat3("translation a", &m_TranslationA.x, 0.0f, WINDW_SIZE_X);
		ImGui::SliderFloat3("translation b", &m_TranslationB.x, 0.0f, WINDW_SIZE_X);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
}