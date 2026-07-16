// ImGui context ownership and per-frame Begin/End lifecycle; Application calls Begin() before
// panels and End() after.
#pragma once
#include "Platform/Layers/Layer.hpp"

// Forward declaration: GLFWwindow is an opaque struct; full definition only needed in .cpp.
struct GLFWwindow;

class ImGuiLayer : public Layer {
public:
    explicit ImGuiLayer(GLFWwindow* window);
    ~ImGuiLayer() override = default;

    void OnAttach() override;
    void OnDetach() override;

    static void Begin();
    static void End();

private:
    GLFWwindow* m_Window;
};
