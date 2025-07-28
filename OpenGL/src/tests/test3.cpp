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
		m_PickingTexture(static_cast<int>(WINDW_SIZE_X), static_cast<int>(WINDW_SIZE_Y)),
		m_CameraPersEnabled(true),
		m_PickingShader("res/shaders/Picking.shader")
	{
		m_MVP = Matx4f::identity();
		m_GlobalTransform = Matx4f::identity();

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
		m_CubeMesh = std::make_shared<Mesh>(cubeVertices, sizeof(cubeVertices) / sizeof(Vertex), cubeIndices, sizeof(cubeIndices) / sizeof(unsigned int));

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

		/*
		std::string dragonPath = "res/models/dragon.obj";
		auto meshDragon = LoadModel(dragonPath);

		m_Dragon = m_Scene.CreateGameObject("Dragon");
		m_Dragon->transform.position = { 75.0f, 0.0f, 0.0f };
		m_Dragon->transform.scale = { 100.0f, 100.0f, 100.0f };
		m_Dragon->transform.rotation = Quaternion(90.0f, 1.0f, 0.0f, 0.0f);
		m_Dragon->AddComponent<MeshRendererComponent>(meshDragon, m_Material3);
		//*/
	}

	test3::~test3() {

	}

	void test3::CalculeMVP() {
		Matx4f view = m_Camera.GetViewMatrix();
		Matx4f projection = m_Camera.GetProjectionMatrix(m_CameraPersEnabled);
		m_GlobalTransform = Matx4f::translation(m_Translation) * Matx4f::rotationY(m_Rotation / 180.0 * M_PI) * Matx4f::scaling(m_Scaling * m_scalar);
		
		m_MVP = projection * view * m_GlobalTransform;
	}

	void test3::OnUpdate(float deltaTime) {
		CalculeMVP(); // Calculate the MVP matrix before rendering
		m_Camera.OnUpdate(deltaTime);
		m_Scene.OnUpdate(deltaTime);
	}

	void test3::OnPick() {
		// Chunk of spaghetti code to handle picking
		m_PickingTexture.EnableWriting();


		// Empty the buffer color and depth
		const GLuint clearColor[4] = { 0, 0, 0, 0 };
		GLCall(glClearBufferuiv(GL_COLOR, 0, clearColor));
		GLCall(glClear(GL_DEPTH_BUFFER_BIT)); // También limpia el buffer de profundidad

		m_PickingShader.Bind();

		// 5. Dibujar cada objeto seleccionable
		unsigned int objectId = 1; // Empezamos los IDs en 1 (0 es para el fondo)
		for (auto& go : m_Scene.GetAllGameObjects()) {
			MeshRendererComponent* meshRenderer = go->GetComponent<MeshRendererComponent>();
			if (meshRenderer) {
				// Asignar el ID al objeto (puedes guardarlo en el propio GameObject)
				go->SetPickingID(objectId);
				m_PickingShader.SetUniform1ui("objectID", go->GetPickingID());

				Matx4f modelMVP;
				if (go->name == "Monkey") {
					modelMVP = Matx4f::translation(go->transform.position) * Matx4f::rotationY(M_PI) * Matx4f::scaling(go->transform.scale);
				}
				else {
					modelMVP = Matx4f::translation(go->transform.position) * Matx4f::scaling(go->transform.scale);
				}
				modelMVP = m_MVP * modelMVP;

				m_PickingShader.SetUniformMat4f("u_MVP", modelMVP);

				// Draw the mesh with the picking shader
				auto& mesh = meshRenderer->m_Mesh;
				m_Renderer.Draw(mesh->GetVAO(), mesh->GetIBO(), m_PickingShader);

				objectId++;
			}
		}

		m_PickingTexture.DisableWriting();
	}

	void test3::OnRender() {
		
		this->OnPick(); // Call picking before rendering

		GLCall(glClearColor(0.7f, 0.5f, 0.5f, 1.0f)); // Set the clear color
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // Clear the color buffer

		m_Grid.OnRender(m_Camera, m_CameraPersEnabled);

		

		int clicked_object_id = -1;
		if (g_MouseState.leftButtonFirstPress) {
			unsigned int mouseX = static_cast<unsigned int>(g_MouseState.lastX);
			unsigned int mouseY = static_cast<unsigned int>(WINDW_SIZE_Y - g_MouseState.lastY);

			std::cout << "Mouse clicked at: (" << mouseX << ", " << mouseY << ")" << std::endl;
			PickingTexture::PixelInfo Pixel = m_PickingTexture.ReadPixel(mouseX, mouseY);
			m_SelectedObjectID = Pixel.ObjectID;
			m_SelectedTriangleID = Pixel.PrimID;
			m_IsVertexSelected = false; // Reset vertex selection
			m_pSelectedObject = nullptr;

			for (auto& go : m_Scene.GetAllGameObjects()) {
				if (go->GetPickingID() == m_SelectedObjectID) {
					m_pSelectedObject = go.get();
					break;
				}
			}

			if (m_pSelectedObject) {
				Vec3 clickedWorldPos = m_PickingTexture.ReadWorldPosition(mouseX, mouseY);

				// Get mesh and transform
				MeshRendererComponent* mrc = m_pSelectedObject->GetComponent<MeshRendererComponent>();

				Matx4f modelMatrix;
				if (m_pSelectedObject->name == "Monkey") {
					modelMatrix = Matx4f::translation(m_pSelectedObject->transform.position) * Matx4f::rotationY(M_PI) * Matx4f::scaling(m_pSelectedObject->transform.scale);
				}
				else {
					modelMatrix = Matx4f::translation(m_pSelectedObject->transform.position) * Matx4f::scaling(m_pSelectedObject->transform.scale);
				}
				modelMatrix = m_GlobalTransform * modelMatrix;

				auto& indices = mrc->m_Mesh->GetIndices();
				auto& vertices = mrc->m_Mesh->GetVertices();

				// Get the 3 vertices of the selected triangle
				unsigned int i0 = indices[m_SelectedTriangleID * 3 + 0];
				unsigned int i1 = indices[m_SelectedTriangleID * 3 + 1];
				unsigned int i2 = indices[m_SelectedTriangleID * 3 + 2];

				Vec3 v0_local = vertices[i0].pos;
				Vec3 v1_local = vertices[i1].pos;
				Vec3 v2_local = vertices[i2].pos;

				// Transform them to world space
				Vec3 v0_world = modelMatrix.transformPoint(v0_local);
				Vec3 v1_world = modelMatrix.transformPoint(v1_local);
				Vec3 v2_world = modelMatrix.transformPoint(v2_local);

				// Find which vertex is closest to the click point
				float d0 = (clickedWorldPos - v0_world).length();
				float d1 = (clickedWorldPos - v1_world).length();
				float d2 = (clickedWorldPos - v2_world).length();

				float min_dist = std::min({ d0, d1, d2 });

				m_IsVertexSelected = true;
				if (min_dist == d0) m_SelectedVertexWorldPos = v0_world;
				else if (min_dist == d1) m_SelectedVertexWorldPos = v1_world;
				else m_SelectedVertexWorldPos = v2_world;

				std::cout << "Selected Vertex Pos: (" << m_SelectedVertexWorldPos.x << ", "
					<< m_SelectedVertexWorldPos.y << ", " << m_SelectedVertexWorldPos.z << ")\n";
			}
		}
		else {
			std::cout << "No object clicked. Clearing selection." << std::endl;
			m_SelectedTriangleID = -1;
		}

		for (auto& go : m_Scene.GetAllGameObjects()) { // Assuming Scene has a method to get all objects
			if (!go->m_IsVisible) continue;

			// Get the required components
			MeshRendererComponent* meshRenderer = go->GetComponent<MeshRendererComponent>();
			if (meshRenderer) {
				auto& material = meshRenderer->m_Material;
				auto& mesh = meshRenderer->m_Mesh;

				material->Bind();

				if (go->GetPickingID() == m_SelectedObjectID) {
					// The object is selected
					material->m_Shader->SetUniform1i("u_IsSelected", 1);
					material->m_Shader->SetUniform4f("u_HighlightColor", 0.0f, 0.0f, 0.55f, 1.0f); // Green
					// Face selection
					material->m_Shader->SetUniform1i("u_IsTriangleSelected", 1);
					material->m_Shader->SetUniform1i("u_SelectedTriangleID", m_SelectedTriangleID);
					material->m_Shader->SetUniform4f("u_TriangleHighlightColor", 0.7f, 0.0f, 0.0f, 1.0f); // Yellow for triangle
				}
				else {
					// The object is not selected
					material->m_Shader->SetUniform1i("u_IsSelected", 0); // false
					material->m_Shader->SetUniform1i("u_IsTriangleSelected", 0);
				}

				Matx4f model;
				// TODO : FIX MODEL ROTATION
				if (go->name == "Monkey") {
					 model = Matx4f::translation(go->transform.position) * Matx4f::rotationY(M_PI) * Matx4f::scaling(go->transform.scale);
				} else { 
					model = Matx4f::translation(go->transform.position) * Matx4f::scaling(go->transform.scale); 
				}
				Matx4f mvp = m_MVP * model;

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