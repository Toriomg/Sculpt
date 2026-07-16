#include "Camera.hpp"

Camera::Camera() {
    RecalculateProjectionMatrix();
    RecalculateViewMatrix();
}

// SETTERS

void Camera::SetProjection(ProjectionType Type) {
    m_ProjectionType = Type;
    RecalculateProjectionMatrix();
}

void Camera::SetPosition(Vec3 const& position) {
    m_Position = position;
    RecalculateViewMatrix();
}

void Camera::SetYaw(float yaw) {
    m_Yaw = yaw;
    RecalculateViewMatrix();
}
void Camera::SetPitch(float pitch) {
    m_Pitch = pitch;
    RecalculateViewMatrix();
}
void Camera::SetRotation(float pitch, float yaw) {
    m_Yaw   = yaw;
    m_Pitch = pitch;
    RecalculateViewMatrix();
}

void Camera::SetViewportSize(float width, float height) {
    m_ViewportWidth  = width;
    m_ViewportHeight = height;
    RecalculateProjectionMatrix();
}

void Camera::SetPerspectiveFOV(float fov) {
    m_PerspectiveFOV = fov;
    RecalculateProjectionMatrix();
}
void Camera::SetOrthographicSize(float size) {
    m_OrthographicSize = size;
    RecalculateProjectionMatrix();
}
void Camera::SetNearClip(float nearClip) {
    m_NearClip = nearClip;
    RecalculateProjectionMatrix();
}

void Camera::SetFarClip(float farClip) {
    m_FarClip = farClip;
    RecalculateProjectionMatrix();
}

// END OF SETTERS

void Camera::RecalculateProjectionMatrix() {
    float aspectRatio = 16.0f / 9.0f;  // Default aspect ratio
    if (m_ViewportHeight > 0) { aspectRatio = m_ViewportWidth / m_ViewportHeight; }
    switch (m_ProjectionType) {
        case Camera::ProjectionType::Perspective:
        {
            m_ProjectionMatrix =
                Matx4f::perspective(m_PerspectiveFOV, aspectRatio, m_NearClip, m_FarClip);
            break;
        }
        case Camera::ProjectionType::Orthographic:
        {
            float const orthoHalfHeight =
                m_OrthographicSize;  // Or m_OrthoHalfHeight, or m_OrthographicSize a la Unity

            float const orthoHalfWidth = orthoHalfHeight * aspectRatio;

            m_ProjectionMatrix =
                Matx4f::orthographic(-orthoHalfWidth, orthoHalfWidth, -orthoHalfHeight,
                                     orthoHalfHeight, m_NearClip, m_FarClip);
            break;
        }
        default: break;
    }
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::RecalculateViewMatrix() {
    Vec3 const worldUp(0.0f, 1.0f, 0.0f);
    Vec3 const baseForward(0.0f, 0.0f, -1.0f);

    float const yawInRadians   = radians(m_Yaw);
    float const pitchInRadians = radians(m_Pitch);

    // Yaw is applied around the world Y axis first so horizontal panning is always level.
    // Pitch is then applied around the local right axis derived from the yawed direction.
    // Swapping this order (pitch then yaw) causes the camera to roll when looking up/down.
    Vec3 const yawedForward = rotateVec3(baseForward, worldUp, yawInRadians);

    Vec3 const rightVector = yawedForward.crossProduct(worldUp).normalize();
    m_Front          = rotateVec3(yawedForward, rightVector, pitchInRadians);

    // Normalize to remove floating-point drift introduced by repeated quaternion rotations.
    m_Front.normalize();
    m_Right = m_Front.crossProduct(worldUp).normalize();
    // Right × Front gives upward in a right-handed system; Front × Right would point downward.
    m_Up = m_Right.crossProduct(m_Front).normalize();

    m_ViewMatrix = Matx4f::lookAt(m_Position, m_Position + m_Front, m_Up);

    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}
