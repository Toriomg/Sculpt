#pragma once

#include <GL/glew.h> // Ensure GLEW is included for OpenGL function loading
#include <GLFW/glfw3.h> // Ensure GLFW is included for window handling
#include <iostream>
#include <string>
#include <functional>
#include <vector>


#include "../Core/Camera.h"
#include "../Math/maths.h"
#include "../Graphics/Buffers/VertexBuffer.h"
#include "../Graphics/Buffers/VertexArray.h"
#include "../Graphics/Buffers/IndexBuffer.h"
#include "../Graphics/Buffers/VertexBufferLayout.h"
#include "../Graphics/Shading/Texture.h"
#include "../Editor/inf_grid.h"
#include "../Graphics/Geometry/Mesh.h"
#include "../Graphics/Shading/Material.h"

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