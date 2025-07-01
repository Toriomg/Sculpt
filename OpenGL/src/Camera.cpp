#include "Camera.h"

static int MARGIN = 20.0f; // Margen para detectar los bordes de la ventana
static float EDGE_SPEED = 40.0f; // Paso de movimiento en los bordes

Camera::Camera(float windowWidth, float windowHeight)
    : m_Position(0.0f, 0.0f, -100.0f),  // Posición inicial de la cámara
    m_Target(0.0f, 0.0f, 1.0f),     // Mirando hacia adelante en el eje Z positivo (local)
    m_Up(0.0f, 1.0f, 0.0f),              // Velocidad de movimiento
    m_FOV(90.0f),
    m_OrthoScale(150.0f),
    m_AspectRatio(16.0f / 9.0f),    // Un valor por defecto
    m_NearClip(0.1f),
    m_FarClip(1000.0f),
    m_MouseSensitivity(0.1f)
{
	m_WindowWidth = windowWidth;
	m_WindowHeight = windowHeight;
	m_AspectRatio = windowWidth / windowHeight; // Calculamos el aspecto de la ventana

    m_Yaw = 100.00f;
    m_Pitch = 0.0f;
    //this->OnUpdate(0.0f);
}

void Camera::SetPosition(const Vec3& position) {
	m_Position = position;
}

void Camera::SetSpeed(float speed) {
	m_Speed = speed;
}

void Camera::OnUpdate(float deltaTime) {
	float deltaYaw = 0.0f;
	float deltaPitch = 0.0f;
    if (m_OnLeftEdge) {
		deltaYaw += EDGE_SPEED * deltaTime; // Y aumenta para mirar a la derecha
	} else if (m_OnRightEdge) {
        deltaYaw -= EDGE_SPEED * deltaTime; // Y disminuye para mirar a la izquierda
	} else if (m_OnUpperEdge) {
        deltaPitch -= EDGE_SPEED * deltaTime; // Y aumenta para mirar arriba
    } else if (m_OnLowerEdge) {
        deltaPitch += EDGE_SPEED * deltaTime; // Y disminuye para mirar abajo
	}
    m_Yaw += deltaYaw;
    m_Pitch += deltaPitch;

    if (m_Pitch > 89.0f) {
        m_Pitch = 89.0f;
    }
    else if (m_Pitch < -89.0f) {
        m_Pitch = -89.0f;
    }

	Vec3 Yaxis(0.0f, 1.0f, 0.0f);

    float yawInRadians = m_Yaw * (3.14159265f / 180.0f);
    float pitchInRadians = m_Pitch * (3.14159265f / 180.0f);

    // yaw rotation
    Vec3 View(1.0f, 0.0f, 0.0f);
    View = rotateVec3(View, Yaxis, yawInRadians);
	View.normalize();

	// pitch rotation
	Vec3 right = View.crossProduct(Yaxis).normalize().normalize();
	View = rotateVec3(View, right, pitchInRadians);

	m_Target = View.normalize();
    m_Up = right.crossProduct(m_Target).normalize();
}

void Camera::OnInput(GLFWwindow* window, float deltaTime) {
    Vec3 worldUp(0.0f, 1.0f, 0.0f);
    Vec3 right = m_Target.crossProduct(worldUp).normalize();

    // Movimiento hacia adelante/atrás
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        m_Position -= m_Target * m_Speed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        m_Position += m_Target * m_Speed * deltaTime;
    }

    // Movimiento lateral (strafe)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        m_Position += right * m_Speed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        m_Position -= right * m_Speed * deltaTime;
    }

    // Movimiento vertical
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        m_Position += m_Up * m_Speed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        m_Position -= m_Up * m_Speed * deltaTime;
    }
}

void Camera::OnMouse(float lastX, float lastY, bool constrainPitch) {
    m_OnLeftEdge = (lastX <= MARGIN);
    m_OnRightEdge = (lastX >= m_WindowWidth - MARGIN);
    m_OnUpperEdge = (lastY <= MARGIN);
    m_OnLowerEdge = (lastY >= m_WindowHeight - MARGIN);

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