#include "EditorCameraController.hpp"
#include "Platform/System/Input/Input.hpp"

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

    if (Input::IsKeyPressed(KeyCode::W))
        currentPosition += m_CameraToControl->GetFrontDirection() * velocity;
    if (Input::IsKeyPressed(KeyCode::S))
        currentPosition -= m_CameraToControl->GetFrontDirection() * velocity;
    if (Input::IsKeyPressed(KeyCode::A))
        currentPosition += m_CameraToControl->GetRightDirection() * velocity;
    if (Input::IsKeyPressed(KeyCode::D))
        currentPosition -= m_CameraToControl->GetRightDirection() * velocity;
    if (Input::IsKeyPressed(KeyCode::LeftShift))
        currentPosition -= m_CameraToControl->GetUpDirection()    * velocity;
    if (Input::IsKeyPressed(KeyCode::Space))
        currentPosition += m_CameraToControl->GetUpDirection()    * velocity;

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
    }
    else
    {
        m_LastMousePosition = Input::GetMousePosition();
    }
}

void EditorCameraController::OnScrolled(float yOffset)
{
    if (!m_CameraToControl) return;

    Vec3 position = m_CameraToControl->GetPosition();
    position += m_CameraToControl->GetFrontDirection() * (yOffset * m_ScrollSpeed);
    m_CameraToControl->SetPosition(position);
}