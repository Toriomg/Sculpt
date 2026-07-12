// Renders the scene framebuffer as an ImGui image; tracks focus/hover state and viewport bounds for input routing.
#pragma once
#include "Editor/Panels/Panel.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"
#include <cstdint>
#include <functional>

class Framebuffer;

class ViewportPanel : public Panel {
public:
    using ResizeCallback = std::function<void(uint32_t, uint32_t)>;

    ViewportPanel(Framebuffer* framebuffer, ResizeCallback onResize);

    void OnImGuiRender() override;

    bool IsFocused()       const { return m_IsFocused; }
    bool IsHovered()       const { return m_IsHovered; }
    Vec2 GetViewportMin()  const { return m_ViewportMin; }

private:
    Framebuffer*   m_Framebuffer;
    ResizeCallback m_OnResize;
    Vec2           m_ViewportMin{0.0f, 0.0f};
    Vec2           m_ViewportSize{0.0f, 0.0f};
    bool           m_IsFocused = false;
    bool           m_IsHovered = false;
};
