#include "EditorLayer.h"

EditorLayer::EditorLayer()
    : Layer("EditorLayer"){
}

void EditorLayer::OnAttach() {
    // This is where you can initialize resources, set up the scene, etc.
    CORE_LOG_INFO("EditorLayer attached!");
}

void EditorLayer::OnUpdate(float deltaTime) {
    // This is where it is updated the game logic every frame
}

void EditorLayer::OnEvent(Event& e) {
    // This layer uses its own dispatcher to handle only the events it cares about.
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseButtonPressedEvent>(std::bind(&EditorLayer::OnMouseButtonPressed, this, std::placeholders::_1));
    dispatcher.Dispatch<MouseButtonReleasedEvent>(std::bind(&EditorLayer::OnMouseButtonReleased, this, std::placeholders::_1));
    dispatcher.Dispatch<KeyPressedEvent>(std::bind(&EditorLayer::OnKeyPressed, this, std::placeholders::_1));
    dispatcher.Dispatch<KeyReleasedEvent>(std::bind(&EditorLayer::OnKeyReleased, this, std::placeholders::_1));
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e) {
    LOG_INFO("Mouse button {0} was pressed!", e.GetMouseButton());
    return false;
}

bool EditorLayer::OnMouseButtonReleased(MouseButtonReleasedEvent& e) {
    LOG_INFO("Mouse button {0} was released!", e.GetMouseButton());
    return false;
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent& e) {
    int keyName = e.GetKeyCode();
    LOG_INFO("Key '{0}' was pressed! (Repeat: {1})", Input::GetKeyName(keyName), e.IsRepeat());
    return false;
}

bool EditorLayer::OnKeyReleased(KeyReleasedEvent& e) {
    LOG_INFO("Key {0} was released!", Input::GetKeyName(e.GetKeyCode()));
    return false;
}