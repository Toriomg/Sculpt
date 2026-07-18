#include "EditorCameraController.hpp"

#include "Platform/System/Input/Input.hpp"
#include <algorithm>

EditorCameraController::EditorCameraController(Camera* camera) : m_CameraToControl(camera) { }

void EditorCameraController::OnUpdate(float deltaTime) {
    if (m_CameraToControl == nullptr) { return; }

    // Keyboard translation is gated on RMB so WASD don't conflict with editor shortcuts.
    if (Input::IsMouseButtonPressed(MouseCode::Right)) {
        float const velocity = m_MovementSpeed * deltaTime;
        Vec3 pos             = m_CameraToControl->GetPosition();

        if (Input::IsKeyPressed(KeyCode::W)) {
            pos += m_CameraToControl->GetFrontDirection() * velocity;
        }
        if (Input::IsKeyPressed(KeyCode::S)) {
            pos -= m_CameraToControl->GetFrontDirection() * velocity;
        }
        if (Input::IsKeyPressed(KeyCode::A)) {
            pos += m_CameraToControl->GetRightDirection() * velocity;
        }
        if (Input::IsKeyPressed(KeyCode::D)) {
            pos -= m_CameraToControl->GetRightDirection() * velocity;
        }
        if (Input::IsKeyPressed(KeyCode::Space)) {
            pos += m_CameraToControl->GetUpDirection() * velocity;
        }
        if (Input::IsKeyPressed(KeyCode::LeftShift)) {
            pos -= m_CameraToControl->GetUpDirection() * velocity;
        }

        m_CameraToControl->SetPosition(pos);

        Vec2 const currentMousePos = Input::GetMousePosition();
        Vec2 delta                 = (currentMousePos - m_LastMousePosition) * m_MouseSensitivity;
        m_LastMousePosition        = currentMousePos;

        float pitch = m_CameraToControl->GetPitch() - delta.y;
        float yaw   = m_CameraToControl->GetYaw() + delta.x;
        pitch       = std::clamp(pitch, -89.0f, 89.0f);
        m_CameraToControl->SetRotation(pitch, yaw);
    } else {
        // Keep last position in sync so re-pressing RMB doesn't jump.
        m_LastMousePosition = Input::GetMousePosition();
    }
}

void EditorCameraController::OnScrolled(float yOffset) {
    if (m_CameraToControl == nullptr) { return; }
    Vec3 pos = m_CameraToControl->GetPosition();
    pos += m_CameraToControl->GetFrontDirection() * (yOffset * m_ScrollSpeed);
    m_CameraToControl->SetPosition(pos);
}
