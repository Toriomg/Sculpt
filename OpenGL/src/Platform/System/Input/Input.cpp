#include "Input.h"
#include "Platform/System/Input/GlfwInput.h"

// Define the static member variable
std::unique_ptr<Input::InputImpl> Input::s_Instance;

// This is where we create the concrete implementation and store it
void Input::Init(Window* window) {
    // We create a new GlfwInput and give it the window it needs to poll
    s_Instance = std::make_unique<GlfwInput>(window);
}

void Input::Shutdown() {
    s_Instance.reset();
}

void Input::OnUpdate() {
    s_Instance->OnUpdateImpl();
}

// --- The public static methods just forward the call to the implementation ---

bool Input::IsKeyPressed(KeyCode key) {
    return s_Instance->IsKeyPressedImpl(key);
}

bool Input::IsMouseButtonPressed(MouseCode button) {
    return s_Instance->IsMouseButtonPressedImpl(button);
}

Vec2 Input::GetMousePosition() {
    return s_Instance->GetMousePositionImpl();
}

float Input::GetMouseX() {
    return GetMousePosition().x;
}

float Input::GetMouseY() {
    return GetMousePosition().y;
}