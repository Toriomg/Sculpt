#pragma once
#include <memory>
#include <array>
#include <GLFW/glfw3.h>

#include "../Scene/Scene.h"
#include "../Scene/GameObject.h"
#include "../Scene/Components/MeshRendererComponent.h" 
#include "../Graphics/Renderer.h"
#include "../Editor/PickingTexture.h"
#include "Tests.h"

struct Vertex3 {
	float Pos[3];
	float Color[4];
	float TexCoords[2];
	float TexID;
};

namespace test {

	class test3 : public Test
	{
	public:
		test3();
		~test3();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnPick();
		void OnImGuiRender() override;
		void OnInput(GLFWwindow* window, float deltaTime) override;

	private:
		Scene m_Scene;
		Renderer m_Renderer;
		std::shared_ptr<VertexArray> m_VAO;
		std::shared_ptr<IndexBuffer> m_IBO;
		std::shared_ptr<Shader> m_Shader;

		std::shared_ptr<Mesh> m_CubeMesh;
		std::shared_ptr<Material> m_Material1;
		std::shared_ptr<Material> m_Material2;
		std::shared_ptr<Material> m_Material3;

		// Pointers to the GameObjects
		GameObject* m_Cube1;
		GameObject* m_Cube2;
		GameObject* m_LargeCube;
		GameObject* m_Cube4;
		GameObject* m_Monkey;
		GameObject* m_Dragon;

		Camera m_Camera;
		InfGrid m_Grid;

		PickingTexture m_PickingTexture;
		Shader m_PickingShader;
		unsigned int m_SelectedObjectID = 0;
		int m_SelectedTriangleID = -1;
		bool m_IsVertexSelected = false;
		Vec3 m_SelectedVertexWorldPos;
		GameObject* m_pSelectedObject = nullptr;

		Matx4f m_MVP;
		Vec3 m_Translation;
		float m_Rotation;
		Vec3 m_Scaling;
		float m_scalar = 1;
		bool m_CameraPersEnabled;

		void CalculeMVP();
	};
};
