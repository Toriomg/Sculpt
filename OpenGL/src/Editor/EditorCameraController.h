#pragma once
#include "Renderer/Camera.h" // It needs to know what a Camera is

class EditorCameraController
{
public:
    // The controller is initialized with the camera it will control.
    EditorCameraController(Camera* camera);

    // This is where the input polling happens.
    void OnUpdate(float deltaTime);

private:
    Camera* m_CameraToControl; // A pointer, does not own the camera

    // Input-related state
    Vec2 m_LastMousePosition = Vec2(0.0f, 0.0f);
    float m_MovementSpeed = 5.0f;
    float m_MouseSensitivity = 1.0f;
};