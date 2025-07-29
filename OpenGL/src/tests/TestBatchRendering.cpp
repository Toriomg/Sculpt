#include <GL/glew.h>
#include <cstring>

#include "TestBatchRendering.h"

#include "imgui/imgui.h"


const float WINDW_SIZE_X = 960.0f; // Define the window width
const float WINDW_SIZE_Y = 540.0f; // Define the window height

namespace test {
	TestBatchRendering::TestBatchRendering()
		:m_Translation(200.0f, 200.0f, 0.0f),
		m_QuadPosition(Vec2(0.0f, 0.0f)),
		m_Proj(Matx4f::orthographic(0.0f, WINDW_SIZE_X, 0.0f, WINDW_SIZE_Y, -10.0f, 10.0f)),
		m_View(Matx4f::translation(Vec3(0.0f, 0.0f, 0.0f)))
	{
		
		/*float positions[] = {
			// I should used array of structs here for better readability and maintainability
			-150.0f, -50.0f, 5.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0, 0.0, 0.0,// Bottom left
			-50.0f,  -50.0f, 5.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0, 0.0, 0.0, // Bottom right
			-50.0f,   50.0f, 5.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0, 1.0, 0.0, // Top right
			-150.0f,  50.0f, 5.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0, 1.0, 0.0, // Top left

			 50.0f, -50.0f, 7.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0, 0.0, 1.0, // Bottom left
			150.0f, -50.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0, 0.0, 1.0, // Bottom right
			150.0f,  50.0f, 7.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0, 1.0, 1.0, // Top right
			 50.0f,  50.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0, 1.0, 1.0 // Top left
		};*/

		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0,

			4 ,5, 6,
			6, 7, 4
		};

		m_VAO = std::make_unique<VertexArray>(); // Create a Vertex Array Object (VAO) to hold the vertex attributes
		m_VBO = std::make_unique<VertexBuffer>(nullptr, static_cast<unsigned int>(sizeof(Vertex)*1024), false); // Create a Vertex Buffer Object (VBO) with the vertex data
		VertexBufferLayout layout;
		layout.Push<float>(3); // position: x, y, z
		layout.Push<float>(4); // color: r, g, b, a
		layout.Push<float>(2); // texture coordinates: u, v
		layout.Push<float>(1); // texture index: used for texture binding

		m_VAO->AddBuffer(*m_VBO, layout);
		m_IBO = std::make_unique<IndexBuffer>(indices, sizeof(indices)); // Create an Index Buffer Object (IBO) with the index data

		// Parse the shader file
		m_Shader = std::make_unique<Shader>("res/shaders/BasicColor.shader"); // Create a Shader object with the shader file path	
		m_Shader->Bind(); // Bind the shader program

		m_Texture1 = std::make_unique<Texture>("res/textures/texture1.png"); // Load the first texture
		m_Texture2 = std::make_unique<Texture>("res/textures/texture2.png"); // Load the second texture
		m_Texture1->Bind(0); // Bind the first texture to texture unit 0
		m_Texture2->Bind(1); // Bind the second texture to texture unit 1

		int samplers[2] = { 0,1 };
		m_Shader->SetUniform1iv("u_Textures", samplers, sizeof(samplers));
	}

	TestBatchRendering::~TestBatchRendering() {

	}

	std::array<VertexStruct, 4> TestBatchRendering::CreateQuad(float x, float y, float textureID){
		float size = 100.0f;

		VertexStruct vertices[] = {
			// Bottom left
			{ {x, y, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, textureID },

			// Bottom right
			{ {x + size, y, 0.0f},  {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, textureID },

			// Top right
			{ {x + size, y + size, 0.0f},   {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, textureID },

			// Top left
			{ {x, y + size, 0.0f},  {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, textureID }
		};
		return { vertices[0], vertices[1], vertices[2], vertices[3] }; // Return the array of vertices
	}

	void TestBatchRendering::OnUpdate(float deltaTime) {
		// Set dynamic vertex buffer

		auto q0 = this->CreateQuad(m_QuadPosition.x, m_QuadPosition.y, 0.0f);
		auto q1 = this->CreateQuad(  50.0f, -50.0f, 1.0f);

		VertexStruct vertices[8];
		memcpy(vertices, q0.data(), q0.size() * sizeof(VertexStruct));
		memcpy(vertices + q0.size(), q1.data(), q1.size() * sizeof(VertexStruct));

		m_VBO->SetData(vertices, sizeof(vertices), 0); // Allocate space for the vertex buffer

	}
	void TestBatchRendering::OnRender() {
		GLCall(glClearColor(0.2f, 0.3f, 0.3f, 1.0f)); // Set the clear color
		GLCall(glClear(GL_COLOR_BUFFER_BIT)); // Clear the color buffer

		RenderCommand renderer; // Create a Renderer object to handle drawing
		{
			Matx4f model = Matx4f::translation(m_Translation);
			Matx4f mvp = m_Proj * m_View * model;
			m_Shader->Bind();
			m_Shader->SetUniformMat4f("u_MVP", mvp);
			// Draw the object using the Renderer
			renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
		}
	}
	void TestBatchRendering::OnImGuiRender() {
		ImGui::Text("Batch Rendering"); // Display text in the ImGui window
		ImGui::DragFloat2("Quad Position", &m_QuadPosition.x, 10);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
}