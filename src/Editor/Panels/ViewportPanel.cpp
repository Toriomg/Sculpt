#include "Editor/Panels/ViewportPanel.hpp"
#include "Platform/Graphics/Framebuffer.hpp"
#include "imgui.h"

ViewportPanel::ViewportPanel(Framebuffer* framebuffer, ResizeCallback onResize)
    : m_Framebuffer(framebuffer), m_OnResize(std::move(onResize)) {}

void ViewportPanel::OnImGuiRender() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
    ImGui::Begin("Viewport");
    ImGui::PopStyleVar();

    m_IsFocused = ImGui::IsWindowFocused();
    m_IsHovered = ImGui::IsWindowHovered();

    // Record the screen-space origin of the content area (used to offset picking coordinates).
    ImVec2 contentPos = ImGui::GetCursorScreenPos();
    m_ViewportMin = Vec2(contentPos.x, contentPos.y);

    ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x > 0.0f && size.y > 0.0f) {
        auto w = static_cast<uint32_t>(size.x);
        auto h = static_cast<uint32_t>(size.y);
        if (w != static_cast<uint32_t>(m_ViewportSize.x) ||
            h != static_cast<uint32_t>(m_ViewportSize.y))
        {
            m_ViewportSize = Vec2(size.x, size.y);
            m_OnResize(w, h);
        }
        uint32_t texID = m_Framebuffer->GetColorAttachmentRendererID();
        // UV is flipped vertically: OpenGL FBOs have origin at bottom-left, ImGui at top-left.
        ImGui::Image(
            static_cast<ImTextureID>(texID),
            size,
            ImVec2{0.0f, 1.0f},
            ImVec2{1.0f, 0.0f}
        );
    }

    ImGui::End();
}
