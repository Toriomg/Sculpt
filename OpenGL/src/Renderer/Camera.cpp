#include "Camera.h"

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
	float aspectRatio = m_ViewportWidth / m_ViewportHeight;

	switch (m_ProjectionType)
	{
		case Camera::ProjectionType::Perspective:{
			m_ProjectionMatrix = Matx4f::perspective(m_PerspectiveFOV, aspectRatio, m_NearClip, m_FarClip);
			break;
		}
		case Camera::ProjectionType::Orthographic: {
			float orthoHalfHeight = m_OrthographicSize; // Or m_OrthoHalfHeight, or m_OrthographicSize a la Unity

			float orthoHalfWidth = orthoHalfHeight * aspectRatio;

			Matx4f::orthographic(-orthoHalfWidth, orthoHalfWidth,
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
	Vec3 front = rotateVec3(yawedForward, rightVector, pitchInRadians);

	// Finalize vectors
	front.normalize();
	Vec3 right = front.crossProduct(worldUp).normalize();
	Vec3 up = right.crossProduct(front).normalize();

	m_ViewMatrix = Matx4f::lookAt(m_Position, m_Position + front, up);

	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}