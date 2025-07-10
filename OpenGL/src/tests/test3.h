#pragma once
#include <memory>
#include <array>
#include <GLFW/glfw3.h>

#include "../Scene/Scene.h"
#include "../Scene/GameObject.h"
#include "../Scene/Components/MeshRendererComponent.h" 
#include "../Graphics/Renderer.h"
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
		void OnImGuiRender() override;
		void OnInput(GLFWwindow* window, float deltaTime) override;

	private:
		Scene m_Scene;
		Renderer m_Renderer;
		std::shared_ptr<VertexBuffer> m_VBO;

		// Pointers to the GameObjects
		GameObject* m_Cube1;
		GameObject* m_Cube2;
		GameObject* m_LargeCube;

		Camera m_Camera;
		InfGrid m_Grid;

		Vec3 m_Translation;
		float m_Rotation;
		Vec3 m_Scaling;
		float m_scalar = 1;
		bool m_CameraPersEnabled = true;
	};
};
