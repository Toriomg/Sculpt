#include <GL/glew.h>
#include <cstring>


#include "test3.h"
#include "../Core/InputManager.h"

#include "imgui/imgui.h"
#include "../Graphics/Loaders/ModelLoader.h"
#include "../Graphics/Geometry/Vertex.h"

extern struct MouseState g_MouseState;

const float WINDW_SIZE_X = 1960.0f*0.75f; // Define the window width
const float WINDW_SIZE_Y = 1080.0f*0.75f; // Define the window height

namespace test {
	test3::test3()
		:m_Translation(0.0f, 0.0f, 0.0f),
		m_Rotation(0.0f),
		m_Scaling(1.0f, 1.0f, 1.0f),
		m_Camera(WINDW_SIZE_X, WINDW_SIZE_Y),
		m_Grid(),
		m_CameraPersEnabled(true)
	{

		auto shader = std::make_shared<Shader>("res/shaders/modelmesh.shader");
		auto shaderMonkey = std::make_shared<Shader>("res/shaders/modelmesh.shader");
		auto texture1 = std::make_shared<Texture>("res/textures/texture1.png");
		auto texture2 = std::make_shared<Texture>("res/textures/texture2.png");

		float s = 1.0f;
		Vertex cubeVertices[] = {
			// positions          // texture coords (u, v, w) // normals (nx, ny, nz)
			// Face 1: Front (+Z)
			Vertex(0.0f, 0.0f, s,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f),
			Vertex(s,    0.0f, s,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f),
			Vertex(s,    s,    s,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f),
			Vertex(0.0f, s,    s,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f),

			// Face 2: Back (-Z)
			Vertex(s,    0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f),
			Vertex(0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f, 0.0f, -1.0f),
			Vertex(0.0f, s,    0.0f,  1.0f, 1.0f, 0.0f, 0.0f, -1.0f),
			Vertex(s,    s,    0.0f,  0.0f, 1.0f, 0.0f, 0.0f, -1.0f),

			// Face 3: Left (-X)
			Vertex(0.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
			Vertex(0.0f, 0.0f, s,     1.0f, 0.0f, -1.0f, 0.0f, 0.0f),
			Vertex(0.0f, s,    s,     1.0f, 1.0f, -1.0f, 0.0f, 0.0f),
			Vertex(0.0f, s,    0.0f,  0.0f, 1.0f, -1.0f, 0.0f, 0.0f),

			// Face 4: Right (+X)
			Vertex(s, 0.0f, s,     0.0f, 0.0f, 1.0f, 0.0f, 0.0f),
			Vertex(s, 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f),
			Vertex(s, s,    0.0f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f),
			Vertex(s, s,    s,     0.0f, 1.0f, 1.0f, 0.0f, 0.0f),

			// Face 5: Top (+Y)
			Vertex(0.0f, s, s,     0.0f, 0.0f, 0.0f, 1.0f, 0.0f),
			Vertex(s,    s, s,     1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
			Vertex(s,    s, 0.0f,  1.0f, 1.0f, 0.0f, 1.0f, 0.0f),
			Vertex(0.0f, s, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f),

			// Face 6: Bottom (-Y)
			Vertex(0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f),
			Vertex(s,    0.0f, 0.0f,  1.0f, 0.0f, 0.0f, -1.0f, 0.0f),
			Vertex(s,    0.0f, s,     1.0f, 1.0f, 0.0f, -1.0f, 0.0f),
			Vertex(0.0f, 0.0f, s,     0.0f, 1.0f, 0.0f, -1.0f, 0.0f)
		};
		unsigned int cubeIndices[] = {
			0, 1, 2,    2, 3, 0,
			4, 5, 6,    6, 7, 4,
			8, 9, 10,   10, 11, 8,
			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20
		};

		// Create ONE mesh, which will be shared by all cubes.
		m_CubeMesh = std::make_shared<Mesh>(cubeVertices, sizeof(cubeVertices), cubeIndices, sizeof(cubeIndices) / sizeof(unsigned int));

		// Create two different materials.
		m_Material1 = std::make_shared<Material>(shader);
		m_Material1->AddTexture(texture1);

		m_Material2 = std::make_shared<Material>(shader);
		m_Material2->AddTexture(texture2);

		m_Material3 = std::make_shared<Material>(shaderMonkey);

		auto multiTextureMaterial = std::make_shared<Material>(shader);
		multiTextureMaterial->AddTexture(texture1);
		multiTextureMaterial->AddTexture(texture2);


		// Create GameObjects and Add Components
		m_Cube1 = m_Scene.CreateGameObject("Small Cube 1");
		m_Cube1->transform.position = { -50.0f, 0.0f, 0.0f };
		m_Cube1->transform.scale = { 10.0f, 10.0f, 10.0f };
		m_Cube1->AddComponent<MeshRendererComponent>(m_CubeMesh, m_Material1);

		m_Cube2 = m_Scene.CreateGameObject("Small Cube 2");
		m_Cube2->transform.position = { -100.0f, 0.0f, 0.0f };
		m_Cube2->transform.scale = { 5.0f, 5.0f, 5.0f };
		m_Cube2->AddComponent<MeshRendererComponent>(m_CubeMesh, m_Material1);

		m_LargeCube = m_Scene.CreateGameObject("Large Center Cube");
		m_LargeCube->transform.position = { -25.0f, -25.0f, -25.0f };
		m_LargeCube->transform.scale = { 50.0f, 50.0f, 50.0f };
		m_LargeCube->AddComponent<MeshRendererComponent>(m_CubeMesh, m_Material1);

		m_Cube4 = m_Scene.CreateGameObject("cube4");
		m_Cube4->transform.position = { -125.0f, 125.0f, 125.0f };
		m_Cube4->transform.scale = { 20.0f, 10.0f, 5.0f };
		m_Cube4->AddComponent<MeshRendererComponent>(m_CubeMesh, m_Material1);
		
		std::string monkeyPath = "res/models/monkey.obj";
		auto meshMonkey = LoadModel(monkeyPath);

		m_Monkey = m_Scene.CreateGameObject("Monkey");
		m_Monkey->transform.position = { 125.0f, 10.0f, 0.0f };
		m_Monkey->transform.scale = { 40.0f, 40.0f, 40.0f };
		m_Monkey->transform.rotation = Quaternion(90.0f, 1.0f, 0.0f, 0.0f);
		m_Monkey->AddComponent<MeshRendererComponent>(meshMonkey, m_Material3);

		
		std::string dragonPath = "res/models/dragon.obj";
		auto meshDragon = LoadModel(dragonPath);

		m_Dragon = m_Scene.CreateGameObject("Dragon");
		m_Dragon->transform.position = { 75.0f, 0.0f, 0.0f };
		m_Dragon->transform.scale = { 100.0f, 100.0f, 100.0f };
		m_Dragon->transform.rotation = Quaternion(90.0f, 1.0f, 0.0f, 0.0f);
		m_Dragon->AddComponent<MeshRendererComponent>(meshDragon, m_Material3);
		
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
		Matx4f global_transform = Matx4f::translation(m_Translation) * Matx4f::rotationY(m_Rotation / 180.0 * M_PI) * Matx4f::scaling(m_Scaling * m_scalar);


		for (auto& go : m_Scene.GetAllGameObjects()) { // Assuming Scene has a method to get all objects
			if (!go->m_IsVisible) continue;

			// Get the required components
			MeshRendererComponent* meshRenderer = go->GetComponent<MeshRendererComponent>();
			if (meshRenderer) {
				auto& material = meshRenderer->m_Material;
				auto& mesh = meshRenderer->m_Mesh;

				material->Bind();
				Matx4f model;
				if (go->name == "Monkey") {
					 model = Matx4f::translation(go->transform.position) * Matx4f::rotationY(M_PI) * Matx4f::scaling(go->transform.scale);
				} else { 
					model = Matx4f::translation(go->transform.position) * Matx4f::scaling(go->transform.scale); 
				}
				Matx4f mvp = projection * view * global_transform * model;

				// 3. Set the MVP uniform (the material already bound the shader)
				material->m_Shader->SetUniformMat4f("u_MVP", mvp);
				material->m_Shader->SetUniformMat4f("u_Model", model);
				material->m_Shader->SetUniform3f("u_cameraPos", m_Camera.m_Position.x, m_Camera.m_Position.y, m_Camera.m_Position.z);

				// 4. Draw the mesh
				m_Renderer.Draw(mesh->GetVAO(), mesh->GetIBO(), *material->m_Shader);
			}
		}
	}

	void test3::OnInput(GLFWwindow* window, float deltaTime) {
		m_Camera.OnInput(window, deltaTime);
		// Process mouse input using the global state from the header
		m_Camera.OnMouse(g_MouseState.x_offset, g_MouseState.y_offset, deltaTime);
		ResetMouseOffset();
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