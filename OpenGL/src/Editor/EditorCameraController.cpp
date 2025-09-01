#include "EditorCameraController.h"
#include "Platform/System/Input/Input.h" // It depends on the Input service
#include "Platform/CoreUtils/Log.h"

EditorCameraController::EditorCameraController(Camera* camera)
    : m_CameraToControl(camera) {
}

void EditorCameraController::OnUpdate(float deltaTime)
{
    // Safety check
    if (!m_CameraToControl) return;

    // --- Keyboard Movement ---
    const float velocity = m_MovementSpeed * deltaTime;
    Vec3 currentPosition = m_CameraToControl->GetPosition();

	Vec3 tempPosition = currentPosition; // For debugging
    if (Input::IsKeyPressed(KeyCode::W))
        currentPosition += m_CameraToControl->GetFrontDirection() * velocity;
    if (Input::IsKeyPressed(KeyCode::S))
        currentPosition -= m_CameraToControl->GetFrontDirection() * velocity;
    if (Input::IsKeyPressed(KeyCode::A))
        currentPosition -= m_CameraToControl->GetRightDirection() * velocity;
    if (Input::IsKeyPressed(KeyCode::D))
        currentPosition += m_CameraToControl->GetRightDirection() * velocity;
    if (Input::IsKeyPressed(KeyCode::Z))
        currentPosition -= m_CameraToControl->GetUpDirection()    * velocity;
    if (Input::IsKeyPressed(KeyCode::X))
        currentPosition += m_CameraToControl->GetUpDirection()    * velocity;

    if (tempPosition.x != currentPosition.x || tempPosition.y != currentPosition.y || tempPosition.z != currentPosition.z)
		LOG_INFO("Camera Moved to: ({0}, {1}, {2})", currentPosition.x, currentPosition.y, currentPosition.z);
    m_CameraToControl->SetPosition(currentPosition);


    // --- Mouse Rotation ---
    if (Input::IsMouseButtonPressed(MouseCode::Right))
    {
        const Vec2 currentMousePos = Input::GetMousePosition();
        Vec2 delta = currentMousePos - m_LastMousePosition;
        m_LastMousePosition = currentMousePos;

        delta *= m_MouseSensitivity;

        float currentPitch = m_CameraToControl->GetPitch();
        float currentYaw = m_CameraToControl->GetYaw();

        currentYaw += delta.x;
        currentPitch -= delta.y; // Invert Y-axis

        // Clamp pitch
        if (currentPitch > 89.0f) currentPitch = 89.0f;
        if (currentPitch < -89.0f) currentPitch = -89.0f;

        m_CameraToControl->SetRotation(currentPitch, currentYaw);
		LOG_INFO("Camera Rotated {0}, {1}", currentPitch, currentYaw);
    }
    else
    {
        m_LastMousePosition = Input::GetMousePosition();
    }
}