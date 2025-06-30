#include "Camera.h"

Camera::Camera() 
	: m_Position(0.0f, 0.0f, -100.0f),  // Posición inicial de la cámara
	m_Target(0.0f, 0.0f, 1.0f),     // Mirando hacia adelante en el eje Z positivo (local)
	m_Up(0.0f, 1.0f, 0.0f),              // Velocidad de movimiento
	m_FOV(90.0f),
    m_OrthoScale(150.0f),
	m_AspectRatio(16.0f / 9.0f),    // Un valor por defecto
	m_NearClip(0.1f),
	m_FarClip(1000.0f),
    m_Yaw(-90.0f), // Yaw is initialized to -90.0 degrees so a yaw of 0.0 results in a direction vector pointing to the right.
    m_Pitch(0.0f),
    m_MouseSensitivity(0.1f)
{

}

void Camera::SetAspectRatio(float aspectRatio) {
	m_AspectRatio = aspectRatio;
}

void Camera::SetPosition(const Vec3& position) {
	m_Position = position;
}

void Camera::SetSpeed(float speed) {
	m_Speed = speed;
}

void Camera::OnInput(GLFWwindow* window, float deltaTime) {
    // Calculamos el vector "derecha" para el movimiento lateral (strafe)
    Vec3 right = m_Up.crossProduct(m_Target).normalize();

    // Movimiento hacia adelante/atrás
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        m_Position += m_Target * m_Speed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        m_Position -= m_Target * m_Speed * deltaTime;
    }

    // Movimiento lateral (strafe)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        m_Position -= right * m_Speed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        m_Position += right * m_Speed * deltaTime;
    }

    // Movimiento vertical
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        m_Position += m_Up * m_Speed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        m_Position -= m_Up * m_Speed * deltaTime;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= m_MouseSensitivity;
    yoffset *= m_MouseSensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    // Constrain pitch to avoid flipping
    if (constrainPitch) {
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;
    }

    // Recalculate the m_Target vector from yaw and pitch
    Vec3 direction;
    direction.x = cos(m_Yaw * M_PI / 180.0f) * cos(m_Pitch * M_PI / 180.0f);
    direction.y = sin(m_Pitch * M_PI / 180.0f);
    direction.z = sin(m_Yaw * M_PI / 180.0f) * cos(m_Pitch * M_PI / 180.0f);
    m_Target = direction.normalize();
}

Matx4f Camera::GetViewMatrix() const {
	return Matx4f::lookAt(m_Position, m_Target, m_Up);
}

Matx4f Camera::GetProjectionMatrix(bool CameraPersEnabled) const {
    if (CameraPersEnabled) {
        return Matx4f::perspective(m_FOV, m_AspectRatio, m_NearClip, m_FarClip);
    }
    else {
        float orthoHeight = m_OrthoScale; // Define the orthographic height
        float orthoWidth = orthoHeight * m_AspectRatio; // Calculate the orthographic width based on the aspect ratio
        return Matx4f::orthographic(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, m_NearClip, m_FarClip);
    }
}