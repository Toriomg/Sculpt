#pragma once

#include <GL/glew.h> // Ensure GLEW is included for OpenGL function loading
#include <GLFW/glfw3.h> // Ensure GLFW is included for window handling
#include <iostream>
#include <string>
#include <functional>
#include <vector>

#include "../Editor/inf_grid.h"
#include "../Editor/SelectionSystem.h"
#include "../Graphics/Rendering/Renderer.h"
#include "../Core/Camera.h"

#include "../Graphics/Shading/Texture.h"
#include "../Scene/Components/MeshRendererComponent.h"

#include "../Scene/Scene.h"
#include "../Scene/GameObject.h"
#include "../Editor/PickingTexture.h"
#include "../Core/InputManager.h"

#include "imgui/imgui.h"
#include "../Graphics/Loaders/ModelLoader.h"

namespace test {

	class Test {
	public:
		Test();
		virtual ~Test();

		virtual void OnUpdate(float deltaTime);
		virtual void OnRender();
		virtual void OnImGuiRender();
		virtual void OnInput(GLFWwindow* window, float deltaTime);
	};

	class TestMenu : public Test {
	public:
		TestMenu(Test*& currentTestPointer);

		void OnImGuiRender() override;

		template<typename T>
		void RegisterTest(const std::string& name) {
			std::cout << "Registering test: " << name << std::endl;

			m_Tests.push_back(std::make_pair(name, []() {return new T(); }));
		}
	private:
		Test*& m_CurrentTest;
		std::vector<std::pair<std::string, std::function<Test*()>>> m_Tests;
	};
};