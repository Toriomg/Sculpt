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
	Camera();

	void SetAspectRatio(float aspectRatio);
	void SetPosition(const Vec3& position);
	void SetSpeed(float speed);
	void OnInput(GLFWwindow* window, float deltaTime);

	Matx4f GetViewMatrix() const;
	Matx4f GetProjectionMatrix(bool CameraPersEnabled) const;
private:
	float m_Speed = 20.0f;
};
