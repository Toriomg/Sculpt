// Scene camera: tracks position and orientation; computes view and projection matrices for Renderer::BeginScene.
#pragma once

#include "Platform/CoreUtils/Math/maths.hpp"
#include "Platform/CoreUtils/Log.hpp"

class Camera {
public:
    enum class ProjectionType { Perspective = 0, Orthographic = 1 };
public:
    Camera();

    // Sets the camera's projection matrix (e.g., when the window is resized).
    void SetProjection(ProjectionType Type);
    ProjectionType GetProjectionType() const { return m_ProjectionType; }

    // --- Accessors ---
    const Matx4f& GetProjectionMatrix() const { return m_ProjectionMatrix; }
    const Matx4f& GetViewMatrix() const { return m_ViewMatrix; }
    const Matx4f& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

    const Vec3& GetPosition() const { return m_Position; }
    void SetPosition(const Vec3& position);

    // Returns the yaw (Y-axis rotation) and pitch (X-axis rotation) in radians.
    float GetYaw() const { return m_Yaw; }
    float GetPitch() const { return m_Pitch; }
    void SetYaw(float yaw);
    void SetPitch(float pitch);
    void SetRotation(float pitch, float yaw);

    // --- Viewport ---
    void SetViewportSize(float width, float height);
	float GetViewportWidth() const { return m_ViewportWidth; }
	float GetViewportHeight() const { return m_ViewportHeight; }
    float GetAspectRatio() const { return m_ViewportWidth/m_ViewportHeight; }

    void SetPerspectiveFOV(float fov);
    float GetPerspectiveFOV() const { return m_PerspectiveFOV; }

    // Orthographic
    void SetOrthographicSize(float size);
    float GetOrthographicSize() const { return m_OrthographicSize; }

    // Clipping planes (used by both)
    void SetNearClip(float nearClip);
    void SetFarClip(float farClip);

    const Vec3& GetFrontDirection() const { return m_Front; }
    const Vec3& GetUpDirection() const { return m_Up; }
    const Vec3& GetRightDirection() const { return m_Right; }
private:
    void RecalculateProjectionMatrix();
    void RecalculateViewMatrix();
private:
    ProjectionType m_ProjectionType = ProjectionType::Perspective;
    Matx4f m_ProjectionMatrix;
    Matx4f m_ViewMatrix;
    Matx4f m_ViewProjectionMatrix;

    /*
    This are temporal values they are intended to be modified after the Camera Constructor
    */
    // Camera Transform
    Vec3 m_Position = { 0.0f, 0.0f, 5.0f }; // Start 5 units back
    Vec3 m_Front, m_Up, m_Right;

    float m_Pitch = 0.0f; // Rotation around X-axis (looking up/down)
    float m_Yaw = 0.0f; // Rotation around Y-axis (looking left/right)

    float m_ViewportWidth = 1280, m_ViewportHeight = 720;

    // Perspective properties
    float m_PerspectiveFOV = 90.0f;

    // Orthographic properties
    float m_OrthographicSize = 10.0f; // Represents the vertical size

    // Shared properties
    float m_NearClip = 0.1f;
    float m_FarClip = 1000.0f;
};