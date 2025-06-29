#include <GL/glew.h>
#include <cstring>

#include "test3.h"
#include "../Renderer.h"

#include "imgui/imgui.h"


const float WINDW_SIZE_X = 1260.0f; // Define the window width
const float WINDW_SIZE_Y = 1080.0f; // Define the window height

namespace test {
	test3::test3()
		:m_Translation(WINDW_SIZE_X / 2.0f, WINDW_SIZE_Y / 2.0f, 0.0f),
		m_Rotation(0.0f),
		m_Scaling(1.0f, 1.0f, 1.0f),
		m_QuadPosition(Vec2(0.0f, 0.0f)),
		m_Proj(Matx4f::orthographic(0.0f, WINDW_SIZE_X, 0.0f, WINDW_SIZE_Y, -1000.0f, 1000.0f)),
		m_View(Matx4f::translation(Vec3(0.0f, 0.0f, 0.0f)))
	{

		/*float positions[] = {
			// I should used array of structs here for better readability and maintainability
			-150.0f, -50.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0, 0.0, 0.0,// Bottom left
			-50.0f,  -50.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0, 0.0, 0.0, // Bottom right
			-50.0f,   50.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0, 1.0, 0.0, // Top right
			-150.0f,  50.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0, 1.0, 0.0, // Top left

			 50.0f, -50.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0, 0.0, 1.0, // Bottom left
			150.0f, -50.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0, 0.0, 1.0, // Bottom right
			150.0f,  50.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0, 1.0, 1.0, // Top right
			 50.0f,  50.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0, 1.0, 1.0 // Top left
		};*/

		unsigned int indices[] = {
			// Define the indices for the cube
			// Front face
			0, 1, 2,
			2, 3, 0,
			// Back face
			4 ,5, 6,
			6, 7, 4,
			// Left face
			0, 3, 7,
			7, 4, 0,
			// Right face
			1, 5, 6,
			6, 2, 1,
			// Top face
			3, 2, 6,
			6, 7, 3,
			// Bottom face
			0, 4, 5,
			5, 1, 0	
		};

		m_VAO = std::make_unique<VertexArray>(); // Create a Vertex Array Object (VAO) to hold the vertex attributes
		m_VBO = std::make_unique<VertexBuffer>(nullptr, static_cast<unsigned int>(sizeof(Vertex3) * 1024), false); // Create a Vertex Buffer Object (VBO) with the vertex data
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

	test3::~test3() {

	}

	std::array<Vertex3, 8> test3::CreateCube(float x, float y, float size) {
		std::array<Vertex3, 8> vertices = {
			Vertex3{ {x, y, 0.0f},				  {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 0.0f },
			Vertex3{ {x + size, y, 0.0f},		  {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, 0.0f },
			Vertex3{ {x + size, y + size, 0.0f},  {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, 0.0f },
			Vertex3{ {x, y + size, 0.0f},		  {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, 0.0f },
			// Back face vertices
			Vertex3{ {x, y, size},				  {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 0.0f },
			Vertex3{ {x + size, y, size},		  {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, 0.0f },
			Vertex3{ {x + size, y + size, size},  {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, 0.0f },
			Vertex3{ {x, y + size, size},		  {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, 0.0f }
		};
		return vertices; // Return the array of vertices
	}

	void test3::OnUpdate(float deltaTime) {
		// Set dynamic vertex buffer
		auto vertices = CreateCube(m_QuadPosition.x, m_QuadPosition.y, 100.0f);

		m_VBO->SetData(vertices.data(), vertices.size() * sizeof(Vertex3), 0);
	}

	void test3::OnRender() {
		GLCall(glClearColor(0.2f, 0.3f, 0.3f, 1.0f)); // Set the clear color
		GLCall(glClear(GL_COLOR_BUFFER_BIT)); // Clear the color buffer

		Renderer renderer; // Create a Renderer object to handle drawing
		{
			Matx4f model = Matx4f::translation(m_Translation) * Matx4f::rotationX(m_Rotation) * Matx4f::scaling(m_Scaling*m_scalar);
			Matx4f mvp = m_Proj * model * m_View;
			m_Shader->Bind();
			m_Shader->SetUniformMat4fm("u_MVP", mvp);
			// Draw the object using the Renderer
			renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
		}
	}
	void test3::OnImGuiRender() {
		ImGui::Text("Batch Rendering"); // Display text in the ImGui window
		ImGui::DragFloat2("Quad Position", &m_QuadPosition.x, 10);
		ImGui::DragFloat3("Camera Translation", &m_Translation.x, 10.0f);
		ImGui::DragFloat("Camera Rotation", &m_Rotation, 0.5f);
		ImGui::DragFloat("Camera Scaling", &m_scalar, 0.01f);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
}