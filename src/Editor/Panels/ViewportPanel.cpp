#include "Editor/Panels/ViewportPanel.hpp"
#include "Platform/Graphics/Framebuffer.hpp"
#include "imgui.h"
#include <array>
#include <cstdio>

ViewportPanel::ViewportPanel(Framebuffer* framebuffer, ResizeCallback onResize)
    : m_Framebuffer(framebuffer), m_OnResize(std::move(onResize)) { }

void ViewportPanel::OnImGuiRender() {
    if (!IsVisible) { return; }
    ImGui::SetNextWindowPos(ImVec2{300.f, 20.f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2{900.f, 700.f}, ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
    ImGui::Begin("Viewport");
    ImGui::PopStyleVar();

    m_IsFocused = ImGui::IsWindowFocused();
    m_IsHovered = ImGui::IsWindowHovered();

    // Record the screen-space origin of the content area (used to offset picking coordinates).
    ImVec2 const contentPos = ImGui::GetCursorScreenPos();
    m_ViewportMin           = Vec2(contentPos.x, contentPos.y);

    ImVec2 const size = ImGui::GetContentRegionAvail();
    if (size.x > 0.0f && size.y > 0.0f) {
        auto w = static_cast<uint32_t>(size.x);
        auto h = static_cast<uint32_t>(size.y);
        if (w != static_cast<uint32_t>(m_ViewportSize.x) ||
            h != static_cast<uint32_t>(m_ViewportSize.y))
        {
            m_ViewportSize = Vec2(size.x, size.y);
            m_OnResize(w, h);
        }
        uint32_t const texID = m_Framebuffer->GetColorAttachmentRendererID();
        // UV is flipped vertically: OpenGL FBOs have origin at bottom-left, ImGui at top-left.
        ImGui::Image(static_cast<ImTextureID>(texID), size, ImVec2{0.0f, 1.0f}, ImVec2{1.0f, 0.0f});

        // FPS overlay — drawn over the viewport image via the window draw list.
        float const fps = ImGui::GetIO().Framerate;
        std::array<char, 32> buf{};
        std::snprintf(buf.data(), buf.size(), "%.0f FPS", fps);  // NOLINT(cert-err33-c)
        ImVec2 const textPos{contentPos.x + 8.0f, contentPos.y + 8.0f};
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddText(textPos, IM_COL32(255, 255, 255, 220), buf.data());
    }

    ImGui::End();
}
