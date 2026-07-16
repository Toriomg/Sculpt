#include "EditorCameraController.hpp"

#include <algorithm>
#include "Platform/System/Input/Input.hpp"

EditorCameraController::EditorCameraController(Camera* camera) : m_CameraToControl(camera) { }

void EditorCameraController::OnUpdate(float deltaTime) {
    // Safety check
    if (m_CameraToControl == nullptr) { return;
}

    // --- Keyboard Movement ---
    float const velocity = m_MovementSpeed * deltaTime;
    Vec3 currentPosition = m_CameraToControl->GetPosition();

    if (Input::IsKeyPressed(KeyCode::W)) {
        currentPosition += m_CameraToControl->GetFrontDirection() * velocity;
}
    if (Input::IsKeyPressed(KeyCode::S)) {
        currentPosition -= m_CameraToControl->GetFrontDirection() * velocity;
}
    if (Input::IsKeyPressed(KeyCode::A)) {
        currentPosition += m_CameraToControl->GetRightDirection() * velocity;
}
    if (Input::IsKeyPressed(KeyCode::D)) {
        currentPosition -= m_CameraToControl->GetRightDirection() * velocity;
}
    if (Input::IsKeyPressed(KeyCode::LeftShift)) {
        currentPosition -= m_CameraToControl->GetUpDirection() * velocity;
}
    if (Input::IsKeyPressed(KeyCode::Space)) {
        currentPosition += m_CameraToControl->GetUpDirection() * velocity;
}

    m_CameraToControl->SetPosition(currentPosition);

    // --- Mouse Rotation ---
    if (Input::IsMouseButtonPressed(MouseCode::Right)) {
        Vec2 const currentMousePos = Input::GetMousePosition();
        Vec2 delta                 = currentMousePos - m_LastMousePosition;
        m_LastMousePosition        = currentMousePos;

        delta *= m_MouseSensitivity;

        float currentPitch = m_CameraToControl->GetPitch();
        float currentYaw   = m_CameraToControl->GetYaw();

        currentYaw += delta.x;
        // Screen Y increases downward, but pitch should increase as we look up.
        currentPitch -= delta.y;

        // Clamped to ±89° rather than ±90° to avoid the lookAt singularity when
        // front and worldUp become parallel (cross-product yields a zero right vector).
        currentPitch = std::min(currentPitch, 89.0f);
        currentPitch = std::max(currentPitch, -89.0f);

        m_CameraToControl->SetRotation(currentPitch, currentYaw);
    } else {
        // Sync last position while RMB is not held so that re-pressing it doesn't
        // produce a large delta from wherever the cursor was when it was released.
        m_LastMousePosition = Input::GetMousePosition();
    }
}

void EditorCameraController::OnScrolled(float yOffset) {
    if (m_CameraToControl == nullptr) { return;
}

    Vec3 position = m_CameraToControl->GetPosition();
    position += m_CameraToControl->GetFrontDirection() * (yOffset * m_ScrollSpeed);
    m_CameraToControl->SetPosition(position);
}
