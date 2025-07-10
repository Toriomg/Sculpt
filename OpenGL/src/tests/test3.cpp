#include <GL/glew.h>
#include <cstring>


#include "test3.h"
#include "../Core/InputManager.h"

#include "imgui/imgui.h"

extern struct MouseState g_MouseState;

const float WINDW_SIZE_X = 1960.0f*0.75f; // Define the window width
const float WINDW_SIZE_Y = 1080.0f*0.75f; // Define the window height

namespace test {
	test3::test3()
		:m_Translation(0.0f, 0.0f, 0.0f),
		m_Rotation(0.0f),
		m_Scaling(1.0f, 1.0f, 1.0f),
		m_Camera(WINDW_SIZE_X, WINDW_SIZE_Y),
		m_Grid()
	{

		auto shader = std::make_shared<Shader>("res/shaders/BasicColor.shader");
		auto texture1 = std::make_shared<Texture>("res/textures/texture1.png");
		auto texture2 = std::make_shared<Texture>("res/textures/texture2.png");

		float s = 1.0f;
		Vertex3 cubeVertices[] = {
			// Positions      // Colors         // Tex Coords  // Tex ID
			// Face 1: Front
			{{0, 0, s}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 0.0f},
			{{s, 0, s}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, 0.0f},
			{{s, s, s}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, 0.0f},
			{{0, s, s}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, 0.0f},
			// Face 2: Back
			{{s, 0, 0}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 1.0f},
			{{0, 0, 0}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, 1.0f},
			{{0, s, 0}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, 1.0f},
			{{s, s, 0}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, 1.0f},
			// Face 3: Left
			{{0, 0, 0}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, 0.0f},
			{{0, 0, s}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, 0.0f},
			{{0, s, s}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, 0.0f},
			{{0, s, 0}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, 0.0f},
			// Face 4: Right
			{{s, 0, s}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 1.0f},
			{{s, 0, 0}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, 1.0f},
			{{s, s, 0}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, 1.0f},
			{{s, s, s}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, 1.0f},
			// Face 5: Top
			{{0, s, s}, {0.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, 0.0f},
			{{s, s, s}, {0.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, 0.0f},
			{{s, s, 0}, {0.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, 0.0f},
			{{0, s, 0}, {0.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, 0.0f},
			// Face 6: Bottom
			{{0, 0, 0}, {1.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, 1.0f},
			{{s, 0, 0}, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, 1.0f},
			{{s, 0, s}, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, 1.0f},
			{{0, 0, s}, {1.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, 1.0f}
		};
		unsigned int cubeIndices[] = {
			0, 1, 2,    2, 3, 0,
			4, 5, 6,    6, 7, 4,
			8, 9, 10,   10, 11, 8,
			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20
		};

		auto vbo = std::make_shared<VertexBuffer>(cubeVertices, sizeof(cubeVertices), false);
		VertexBufferLayout layout;
		layout.Push<float>(3); // position: x, y, z
		layout.Push<float>(4); // color: r, g, b, a
		layout.Push<float>(2); // texture coordinates: u, v
		layout.Push<float>(1); // texture index: used for texture binding

		// Create Vertex Array Object (VAO) and Index Buffer Object (IBO)
		auto vao = std::make_shared<VertexArray>();
		vao->AddBuffer(*vbo, layout);
		auto ibo = std::make_shared<IndexBuffer>(cubeIndices, sizeof(cubeIndices) / sizeof(unsigned int));

		// Create GameObjects and Add Components
		m_Cube1 = m_Scene.CreateGameObject("Small Cube 1");
		m_Cube1->transform.position = { 50.0f, 0.0f, 0.0f };
		m_Cube1->transform.scale = { 10.0f, 10.0f, 10.0f };
		auto& mrc1 = *m_Cube1->AddComponent<MeshRendererComponent>(vao, ibo, shader);
		mrc1.AddTexture(texture1);

		m_Cube2 = m_Scene.CreateGameObject("Small Cube 2");
		m_Cube2->transform.position = { 100.0f, 0.0f, 0.0f };
		m_Cube2->transform.scale = { 5.0f, 5.0f, 5.0f };
		auto& mrc2 = *m_Cube2->AddComponent<MeshRendererComponent>(vao, ibo, shader); // RE-USING the same mesh and shader!
		mrc2.AddTexture(texture2);

		m_LargeCube = m_Scene.CreateGameObject("Large Center Cube");
		m_LargeCube->transform.position = { -25.0f, -25.0f, -25.0f };
		m_LargeCube->transform.scale = { 50.0f, 50.0f, 50.0f };
		auto& mrc3 = *m_LargeCube->AddComponent<MeshRendererComponent>(vao, ibo, shader); // RE-USING again!
		mrc3.AddTexture(texture1);
		mrc3.AddTexture(texture2);
	}

	test3::~test3() {

	}

	void test3::OnUpdate(float deltaTime) {
		m_Camera.OnUpdate(deltaTime);
		m_Scene.OnUpdate(deltaTime);
	}

	void test3::OnRender() {
		GLCall(glClearColor(0.7f, 0.5f, 0.5f, 1.0f)); // Set the clear color
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // Clear the color buffer

		m_Grid.OnRender(m_Camera, m_CameraPersEnabled);

		Matx4f view = m_Camera.GetViewMatrix();
		Matx4f projection = m_Camera.GetProjectionMatrix(m_CameraPersEnabled);

		std::cout << "debug line1" << std::endl;
		for (auto& go : m_Scene.GetAllGameObjects()) { // Assuming Scene has a method to get all objects
			std::cout << "debug line2" << std::endl;
			if (!go->m_IsVisible) continue;

			// Get the required components
			MeshRendererComponent* meshRenderer = go->GetComponent<MeshRendererComponent>();
			if (meshRenderer) {
				meshRenderer->m_Shader->Bind();
				std::cout << "debug line3" << std::endl;
				// Bind textures
				int samplers[2] = { 0, 1 }; // Max textures
				for (int i = 0; i < meshRenderer->Textures.size(); ++i) {
					meshRenderer->Textures[i]->Bind(i);
				}
				meshRenderer->m_Shader->SetUniform1iv("u_Textures", samplers, meshRenderer->Textures.size());

				Matx4f model = Matx4f::translation(m_Translation) * Matx4f::rotationY(m_Rotation) * Matx4f::scaling(m_Scaling * m_scalar);

				// 3. Set the MVP uniform
				Matx4f mvp = projection * view * model;
				meshRenderer->m_Shader->SetUniformMat4f("u_MVP", mvp);
				std::cout << "debug line4" << std::endl;
				// 4. Draw
				m_Renderer.Draw(*meshRenderer->m_VAO, *meshRenderer->m_IBO, *meshRenderer->m_Shader);
				std::cout << "debug line5" << std::endl;
			}
		}
	}

	void test3::OnInput(GLFWwindow* window, float deltaTime) {
		m_Camera.OnInput(window, deltaTime);
		// Process mouse input using the global state from the header
		m_Camera.OnMouse(g_MouseState.lastX, g_MouseState.lastY);
	}

	void test3::OnImGuiRender() {
		m_Camera.OnImGuiRender(m_CameraPersEnabled);

		ImGui::Text("Scene Object Controls");
		// These controls are specific to this test scene
		ImGui::DragFloat3("Cube 1 Position", &m_Cube1->transform.position.x, 1.0f);
		ImGui::DragFloat3("Cube 2 Position", &m_Cube2->transform.position.x, 1.0f);

		ImGui::Separator();
		ImGui::Text("Global Transform Controls");
		ImGui::DragFloat3("Model Translation", &m_Translation.x, 0.1f);
		ImGui::DragFloat("Model Rotation", &m_Rotation, 0.5f);
		ImGui::DragFloat3("Model Scaling", &m_Scaling.x, 0.01f);
		ImGui::DragFloat("Scalar Multiplier", &m_scalar, 0.01f);

		ImGui::Separator();
		ImGui::Text("Performance");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
}