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

void Camera::SetPosition(const Vec3& position) {
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
	m_Yaw = yaw;
	m_Pitch = pitch;
	RecalculateViewMatrix();
}

void Camera::SetViewportSize(float width, float height) {
	m_ViewportWidth = width;
	m_ViewportHeight = height;
	RecalculateProjectionMatrix();
};

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
	float aspectRatio = 16.0f / 9.0f; // Default aspect ratio
	if (m_ViewportHeight > 0) {
		aspectRatio = m_ViewportWidth / m_ViewportHeight;
	}
	switch (m_ProjectionType)
	{
		case Camera::ProjectionType::Perspective:{
			m_ProjectionMatrix = Matx4f::perspective(m_PerspectiveFOV, aspectRatio, m_NearClip, m_FarClip);
			break;
		}
		case Camera::ProjectionType::Orthographic: {
			float orthoHalfHeight = m_OrthographicSize; // Or m_OrthoHalfHeight, or m_OrthographicSize a la Unity

			float orthoHalfWidth = orthoHalfHeight * aspectRatio;

			m_ProjectionMatrix = Matx4f::orthographic(-orthoHalfWidth, orthoHalfWidth,
				-orthoHalfHeight, orthoHalfHeight,
				m_NearClip, m_FarClip);
			break;
		}
		default:
			break;
	}
	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::RecalculateViewMatrix() {
	const Vec3 worldUp(0.0f, 1.0f, 0.0f);
	const Vec3 baseForward(0.0f, 0.0f, -1.0f);

	float yawInRadians = radians(m_Yaw);
	float pitchInRadians = radians(m_Pitch);

	// Apply yaw
	Vec3 yawedForward = rotateVec3(baseForward, worldUp, yawInRadians);

	// Calculate right vector and apply pitch
	Vec3 rightVector = yawedForward.crossProduct(worldUp).normalize();
	m_Front = rotateVec3(yawedForward, rightVector, pitchInRadians);

	// Finalize vectors
	m_Front.normalize();
	m_Right = m_Front.crossProduct(worldUp).normalize();
	//this is for inverted
	//m_Up = m_Front.crossProduct(m_Right).normalize(); 
	m_Up = m_Right.crossProduct(m_Front).normalize();

	m_ViewMatrix = Matx4f::lookAt(m_Position, m_Position + m_Front, m_Up);

	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}