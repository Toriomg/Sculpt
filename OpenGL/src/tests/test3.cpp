#include <GL/glew.h>
#include <cstring>


#include "test3.h"
#include "../Renderer.h"
#include "../InputManager.h"

#include "imgui/imgui.h"

extern struct MouseState g_MouseState;

const float WINDW_SIZE_X = 1960.0f*0.75f; // Define the window width
const float WINDW_SIZE_Y = 1080.0f*0.75f; // Define the window height

namespace test {
	test3::test3()
		:m_Translation(0.0f, 0.0f, 0.0f),
		m_Rotation(0.0f),
		m_Scaling(1.0f, 1.0f, 1.0f),

		m_QuadPosition(Vec3(50.0f, 0.0f, 0.0f)),
		m_QuadPosition2(Vec3(100.0f, 0.0f, 0.0f)),
		m_Camera(WINDW_SIZE_X, WINDW_SIZE_Y)
	{
		
		const unsigned int indices[] = {
			// Cube 0 (Original)
			0, 1, 2,    2, 3, 0,    // Front
			4, 5, 6,    6, 7, 4,    // Back
			8, 9, 10,   10, 11, 8,  // Left
			12, 13, 14, 14, 15, 12, // Right
			16, 17, 18, 18, 19, 16, // Top
			20, 21, 22, 22, 23, 20, // Bottom

			// Cube 1 (Offset = +24)
			24, 25, 26,    26, 27, 24,
			28, 29, 30,    30, 31, 28,
			32, 33, 34,    34, 35, 32,
			36, 37, 38,    38, 39, 36,
			40, 41, 42,    42, 43, 40,
			44, 45, 46,    46, 47, 44,

			// Cube 2 (Offset = +48)
			48, 49, 50,    50, 51, 48,
			52, 53, 54,    54, 55, 52,
			56, 57, 58,    58, 59, 56,
			60, 61, 62,    62, 63, 60,
			64, 65, 66,    66, 67, 64,
			68, 69, 70,    70, 71, 68
		};
		

		m_VAO = std::make_unique<VertexArray>(); // Create a Vertex Array Object (VAO) to hold the vertex attributes
		m_VBO = std::make_unique<VertexBuffer>(nullptr, static_cast<unsigned int>(sizeof(Vertex3) * 1024), false); // Create a Vertex Buffer Object (VBO) with the vertex data
		VertexBufferLayout layout;
		layout.Push<float>(3); // position: x, y, z
		layout.Push<float>(4); // color: r, g, b, a
		layout.Push<float>(2); // texture coordinates: u, v
		layout.Push<float>(1); // texture index: used for texture binding

		m_VAO->AddBuffer(*m_VBO, layout);
		m_IBO = std::make_unique<IndexBuffer>(indices,
			sizeof(indices)); // Create an Index Buffer Object (IBO) with the index data

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

	std::array<Vertex3, 24> test3::CreateCube(float x, float y, float z, float size) {
		// We define 24 vertices, 4 for each of the 6 faces.
		// This allows each face to have its own color, texture, and normal vector.
		std::array<Vertex3, 24> vertices = {
			// Face 1: Front (looking towards positive Z) - Red, Texture 0
			Vertex3{ {x,        y,        z + size}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 0.0f }, // Bottom-left
			Vertex3{ {x + size, y,        z + size}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, 0.0f }, // Bottom-right
			Vertex3{ {x + size, y + size, z + size}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, 0.0f }, // Top-right
			Vertex3{ {x,        y + size, z + size}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, 0.0f }, // Top-left

			// Face 2: Back (looking towards negative Z) - Green, Texture 1
			Vertex3{ {x + size, y,        z       }, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 1.0f }, // Bottom-left
			Vertex3{ {x,        y,        z       }, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, 1.0f }, // Bottom-right
			Vertex3{ {x,        y + size, z       }, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, 1.0f }, // Top-right
			Vertex3{ {x + size, y + size, z       }, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, 1.0f }, // Top-left

			// Face 3: Left (looking towards negative X) - Blue
			Vertex3{ {x,        y,        z       }, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, 0.0f }, // Bottom-left
			Vertex3{ {x,        y,        z + size}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, 0.0f }, // Bottom-right
			Vertex3{ {x,        y + size, z + size}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, 0.0f }, // Top-right
			Vertex3{ {x,        y + size, z       }, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, 0.0f }, // Top-left

			// Face 4: Right (looking towards positive X) - Yellow
			Vertex3{ {x + size, y,        z + size}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 1.0f }, // Bottom-left
			Vertex3{ {x + size, y,        z       }, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, 1.0f }, // Bottom-right
			Vertex3{ {x + size, y + size, z       }, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, 1.0f }, // Top-right
			Vertex3{ {x + size, y + size, z + size}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, 1.0f }, // Top-left

			// Face 5: Top (looking towards positive Y) - Cyan
			Vertex3{ {x,        y + size, z + size}, {0.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, 0.0f }, // Bottom-left
			Vertex3{ {x + size, y + size, z + size}, {0.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, 0.0f }, // Bottom-right
			Vertex3{ {x + size, y + size, z       }, {0.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, 0.0f }, // Top-right
			Vertex3{ {x,        y + size, z       }, {0.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, 0.0f }, // Top-left

			// Face 6: Bottom (looking towards negative Y) - Magenta
			Vertex3{ {x,        y,        z       }, {1.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, 1.0f }, // Bottom-left
			Vertex3{ {x + size, y,        z       }, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, 1.0f }, // Bottom-right
			Vertex3{ {x + size, y,        z + size}, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, 1.0f }, // Top-right
			Vertex3{ {x,        y,        z + size}, {1.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, 1.0f }  // Top-left
		};
		return vertices;
	}

	void test3::OnUpdate(float deltaTime) {
		m_Camera.OnUpdate(deltaTime, g_MouseState); // Update the camera state based on input and delta time

		// Set dynamic vertex buffer
		auto q0 = CreateCube(m_QuadPosition.x, m_QuadPosition.y, m_QuadPosition.z, 10);   // Cubo pequeño 1
		auto q1 = CreateCube(-25, -25, -25, 50); // Cubo GRANDE (origen del triángulo)
		auto q2 = CreateCube(m_QuadPosition2.x, m_QuadPosition2.y, m_QuadPosition2.z, 5);  // Cubo pequeño 2 (fuera de vista)
		
		Vertex3 vertices[24*3];

		memcpy(vertices, q0.data(), q0.size() * sizeof(Vertex3));
		memcpy(vertices + q0.size(), q1.data(), q1.size() * sizeof(Vertex3));
		memcpy(vertices + q0.size() + q1.size(), q2.data(), q2.size() * sizeof(Vertex3));

		m_VBO->SetData(vertices, sizeof(vertices), 0);
		//m_VBO->SetData(vertices.data(), vertices.size() * sizeof(Vertex3), 0);
	}

	void test3::OnRender() {
		GLCall(glClearColor(0.2f, 0.3f, 0.3f, 1.0f)); // Set the clear color
		GLCall(glClear(GL_COLOR_BUFFER_BIT)); // Clear the color buffer

		Matx4f model = Matx4f::translation(m_Translation) * Matx4f::rotationY(m_Rotation) * Matx4f::scaling(m_Scaling * m_scalar);
		// Rotated to see the positive Z direction
		Matx4f view = m_Camera.GetViewMatrix();
		Matx4f projection = m_Camera.GetProjectionMatrix(m_CameraPersEnabled);

		Renderer renderer; // Create a Renderer object to handle drawing
		{

			Matx4f mvp = projection * view * model;
			m_Shader->Bind();
			m_Shader->SetUniformMat4fm("u_MVP", mvp);
			// Draw the object using the Renderer
			renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
		}
	}

	void test3::OnInput(GLFWwindow* window, float deltaTime) {
		m_Camera.OnInput(window, deltaTime);
		// Process mouse input using the global state from the header
		m_Camera.OnMouse(g_MouseState.x_offset, g_MouseState.y_offset);

		// Reset the offset
		g_MouseState.x_offset = 0.0f;
		g_MouseState.y_offset = 0.0f;

		// A key to release the mouse cursor (e.g., to use the ImGui menu)
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		// A key to re-capture the mouse
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			// Only recapture if the ImGui window is not being hovered
			if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered()) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
	}

	void test3::OnImGuiRender() {
		ImGui::DragFloat3("Camera TARGET", &m_Camera.m_Target.x, 5.0f);
		ImGui::DragFloat3("Camera UP", &m_Camera.m_Up.x, 5.0f);
		ImGui::DragFloat("Yaw", &m_Camera.m_Yaw, 0.01f);
		ImGui::DragFloat("Pitch", &m_Camera.m_Pitch, 0.01f);
		ImGui::Separator();
		ImGui::DragFloat3("Quad Position", &m_QuadPosition.x, 10);
		ImGui::DragFloat3("Quad Position 2", &m_QuadPosition2.x, 10);
		ImGui::Separator();
		ImGui::Text("\nCamera Transformations");
		ImGui::DragFloat3("Camera Translation", &m_Camera.m_Position.x, 5.0f);
		ImGui::DragFloat("Transform Rotation", &m_Rotation, 0.5f);
		ImGui::DragFloat("Transform Scalar", &m_scalar, 0.01f);
		ImGui::DragFloat3("Transform Scaling", &m_Scaling.x, 0.01f);
		ImGui::Separator();
		ImGui::Text("\nCamera Settings");
		ImGui::Checkbox("Camera Perspective Enabled", &m_CameraPersEnabled);
		if (m_CameraPersEnabled) {
			ImGui::Text("Camera Perspective Mode Enabled");
			ImGui::DragFloat("Camera FOV", &m_Camera.m_FOV, 0.5f);
		} else {
			ImGui::Text("Camera Orthographic Mode Enabled");
			ImGui::DragFloat("Ortho Scale (Zoom)", &m_Camera.m_OrthoScale, 1.0f, 1.0f, 1000.0f);
		}
		ImGui::DragFloat("Camera Near Clip", &m_Camera.m_NearClip, 0.1f);
		ImGui::DragFloat("Camera Far Clip", &m_Camera.m_FarClip, 10.0f);
		ImGui::DragFloat("Camera Speed", &m_Camera.m_Speed, 5.0f);
		ImGui::Separator();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
}