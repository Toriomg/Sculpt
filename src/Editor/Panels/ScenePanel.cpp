#include "Editor/Panels/ScenePanel.hpp"
#include "Renderer/Camera.hpp"
#include "imgui.h"

ScenePanel::ScenePanel(Camera* camera) : m_Camera(camera) { }

Matx4f ScenePanel::GetGlobalTransform() const {
    return Matx4f::translation(m_Translation) *
           Matx4f::rotationY(m_Rotation) *
           Matx4f::scaling(m_Scaling) *
           Matx4f::scalingScalar(m_Scalar);
}

Matx4f ScenePanel::GetInverseGlobalTransform() const {
    // Inverse of (T * Ry * Speraxis * Suniform) is (Suniform_inv * Speraxis_inv * Ry_inv * T_inv).
    float safeScalar = (m_Scalar != 0.f) ? 1.f / m_Scalar : 1.f;
    Vec3 safeScale{
      (m_Scaling.x != 0.f) ? 1.f / m_Scaling.x : 1.f,
      (m_Scaling.y != 0.f) ? 1.f / m_Scaling.y : 1.f,
      (m_Scaling.z != 0.f) ? 1.f / m_Scaling.z : 1.f,
    };
    return Matx4f::scalingScalar(safeScalar) *
           Matx4f::scaling(safeScale) *
           Matx4f::rotationY(-m_Rotation) *
           Matx4f::translation({-m_Translation.x, -m_Translation.y, -m_Translation.z});
}

void ScenePanel::OnImGuiRender() {
    if (!IsVisible) return;
    ImGui::SetNextWindowPos(ImVec2{10.f, 20.f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2{280.f, 450.f}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Scene");

    ImGui::Text("Global Transform Controls");
    ImGui::DragFloat3("Model Translation", &m_Translation.x, 0.1f);
    ImGui::DragFloat("Model Rotation", &m_Rotation, 0.5f);
    ImGui::DragFloat3("Model Scaling", &m_Scaling.x, 0.01f, 0.001f, 100.0f);
    ImGui::DragFloat("Scalar Multiplier", &m_Scalar, 0.01f, 0.001f, 100.0f);

    ImGui::Separator();

    if (!m_Camera) {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto projType = m_Camera->GetProjectionType();
        if (ImGui::RadioButton("Perspective", projType == Camera::ProjectionType::Perspective))
            m_Camera->SetProjection(Camera::ProjectionType::Perspective);
        ImGui::SameLine();
        if (ImGui::RadioButton("Orthographic", projType == Camera::ProjectionType::Orthographic))
            m_Camera->SetProjection(Camera::ProjectionType::Orthographic);

        if (projType == Camera::ProjectionType::Perspective) {
            float fov = m_Camera->GetPerspectiveFOV();
            if (ImGui::DragFloat("FOV", &fov, 0.5f, 1.0f, 179.0f)) m_Camera->SetPerspectiveFOV(fov);
        } else {
            float size = m_Camera->GetOrthographicSize();
            if (ImGui::DragFloat("Ortho Size", &size, 0.1f, 0.1f, 1000.0f))
                m_Camera->SetOrthographicSize(size);
        }

        float yaw = m_Camera->GetYaw();
        if (ImGui::DragFloat("Yaw", &yaw, 0.5f)) m_Camera->SetYaw(yaw);

        float pitch = m_Camera->GetPitch();
        if (ImGui::DragFloat("Pitch", &pitch, 0.5f, -89.0f, 89.0f)) m_Camera->SetPitch(pitch);

        Vec3 pos = m_Camera->GetPosition();
        if (ImGui::DragFloat3("Position##cam", &pos.x, 0.1f)) m_Camera->SetPosition(pos);
    }

    ImGui::End();
}
