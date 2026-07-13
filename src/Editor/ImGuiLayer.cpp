#include "Editor/ImGuiLayer.hpp"
#include "Editor/UIStyle.hpp"
#include "Platform/Graphics/RenderCommand.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

ImGuiLayer::ImGuiLayer(GLFWwindow* window)
    : Layer("ImGuiLayer"), m_Window(window) {}

void ImGuiLayer::OnAttach() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    UIStyle::ApplyTheme(UIStyle::Theme::ModelingDark);
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void ImGuiLayer::OnDetach() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::Begin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // Clear the screen so stale content from the previous frame doesn't bleed through
    // the semi-transparent areas of ImGui windows.
    RenderCommand::SetClearColor({0.15f, 0.15f, 0.15f, 1.0f});
    RenderCommand::Clear();
}

void ImGuiLayer::End() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
