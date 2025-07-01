#include "Camera.h"

static int MARGIN = 10; // Margen para detectar los bordes de la ventana
static float EDGE_SPEED = 50.0f; // Paso de movimiento en los bordes

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

    Vec3 Targetnew = Vec3(m_Target.x, 0.0f, m_Target.z).normalize();
	// Calculamos el ángulo de yaw en radianes y lo convertimos a grados
	float yaw_radians = atan2(Targetnew.x, Targetnew.z); // atan2(y, x) gives the angle in radians
    m_Yaw = yaw_radians * 180.0f / M_PI;
    if (m_Yaw < 0.0f) {
        m_Yaw += 360.0f;
    }
     
    m_Pitch = 0;
}

void Camera::SetPosition(const Vec3& position) {
	m_Position = position;
}

void Camera::SetSpeed(float speed) {
	m_Speed = speed;
}

void Camera::OnUpdate(float deltaTime, const MouseState& mouseState) {
	float deltaYaw = 0.0f;
	float deltaPitch = 0.0f;

    if (mouseState.lastX <= MARGIN) deltaYaw -= EDGE_SPEED * deltaTime;
    if (mouseState.lastX >= m_WindowWidth - MARGIN) deltaYaw += EDGE_SPEED * deltaTime;
    if (mouseState.lastY <= MARGIN) deltaPitch += EDGE_SPEED * deltaTime; // Y aumenta para mirar arriba
    if (mouseState.lastY >= m_WindowHeight - MARGIN) deltaPitch -= EDGE_SPEED * deltaTime; // Y disminuye para mirar abajo

    if (deltaYaw != 0.0f || deltaPitch != 0.0f) {
        m_Yaw += deltaYaw;
        m_Pitch += deltaPitch;
        if (m_Pitch > 89.0f) {
            m_Pitch = 89.0f;
        }
        else if (m_Pitch < -89.0f) {
            m_Pitch = -89.0f;
        }
    }

	Vec3 Yaxis(0.0f, 1.0f, 0.0f);

    // yaw rotation
    Vec3 View(1.0f, 0.0f, 0.0f);
    View = rotateVec3(View, Yaxis, m_Yaw);
	View.normalize();

	// pitch rotation
	Vec3 right = Yaxis.crossProduct(View).normalize();
	View = rotateVec3(View, right, m_Pitch);

	m_Target = View.normalize();
    m_Up = m_Target.crossProduct(right).normalize();
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

void Camera::OnMouse(float xoffset, float yoffset, bool constrainPitch) {
    m_Yaw += xoffset * m_MouseSensitivity;
    m_Pitch += yoffset * m_MouseSensitivity;
}

Matx4f Camera::GetViewMatrix() const {
	return Matx4f::lookAt(m_Position, m_Position + m_Target, m_Up);
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