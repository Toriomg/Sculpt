#pragma once
#include "maths/maths.h"
#include <GLFW/glfw3.h> // Necesario para el control de teclado
#include "InputManager.h"

class Camera {
public:
	Vec3 m_Position;
	Vec3 m_Target;
	Vec3 m_Up;

	float m_WindowWidth;
	float m_WindowHeight;

	float m_FOV;
	float m_AspectRatio;
	float m_NearClip;
	float m_FarClip;
	float m_OrthoScale;
	float m_Speed = 100.0f;
	float m_Yaw; // Rotación alrededor del eje Y
	float m_Pitch; // Rotación alrededor del eje X
	float m_MouseSensitivity;

	//flags
	bool m_OnUpperEdge = false;
	bool m_OnLowerEdge = false;
	bool m_OnLeftEdge = false;
	bool m_OnRightEdge = false;
	Camera(float windowWidth, float windowHeight);

	void SetPosition(const Vec3& position);
	void SetSpeed(float speed);
	void OnUpdate(float deltaTime, const MouseState& mouseState);
	void OnInput(GLFWwindow* window, float deltaTime);
	void OnMouse(float xoffset, float yoffset, bool constrainPitch = true);

	Matx4f GetViewMatrix() const;
	Matx4f GetProjectionMatrix(bool CameraPersEnabled) const;
private:
};
