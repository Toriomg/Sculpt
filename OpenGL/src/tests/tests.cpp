#include "Tests.h"
#include "imgui/imgui.h"

namespace test {

	Test::Test() {
	}

	Test::~Test() {
	}

	void Test::OnUpdate(float deltaTime) {
	}

	void Test::OnRender() {
	}

	void Test::OnImGuiRender() {
	}

	void Test::OnInput(GLFWwindow* window, float deltaTime) {
	}

	TestMenu::TestMenu(Test*& currentTestPointer)
		: m_CurrentTest(currentTestPointer) {

	}

	void TestMenu::OnImGuiRender() {
		for (auto& test : m_Tests) {
			if (ImGui::Button(test.first.c_str())) {
				m_CurrentTest = test.second();
			}
		}
	}
}