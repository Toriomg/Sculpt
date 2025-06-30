#pragma once
#include "maths/maths.h"
#include <GLFW/glfw3.h> // Necesario para el control de teclado

class Camera {
public:
	Vec3 m_Position;
	Vec3 m_Target;
	Vec3 m_Up;

	float m_FOV;
	float m_AspectRatio;
	float m_NearClip;
	float m_FarClip;
	float m_OrthoScale;
	float m_Speed = 100.0f;
	float m_Yaw;
	float m_Pitch;
	float m_MouseSensitivity;
	Camera();

	void SetAspectRatio(float aspectRatio);
	void SetPosition(const Vec3& position);
	void SetSpeed(float speed);
	void OnInput(GLFWwindow* window, float deltaTime);
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

	Matx4f GetViewMatrix() const;
	Matx4f GetProjectionMatrix(bool CameraPersEnabled) const;
private:
};
