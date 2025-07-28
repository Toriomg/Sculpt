#include "GameObject.h"

GameObject::GameObject(const std::string& name)
    : name(name) {
}

void GameObject::OnUpdate(float deltaTime) {
    if (!m_IsVisible) return;

    // Update all components attached to this GameObject
    for (const auto& component : m_Components) {
        component->OnUpdate(deltaTime);
    }
}

void GameObject::OnRender() {
    if (!m_IsVisible) return;

    // Render all components
    for (const auto& component : m_Components) {
        component->OnRender();
    }
}

Matx4f GameObject::GetTransformMatrix() const {
    Matx4f model;
    if (this->name == "Monkey") {
        model = Matx4f::translation(this->transform.position) * Matx4f::rotationY(M_PI) * Matx4f::scaling(this->transform.scale);
    }
    else {
        model = Matx4f::translation(this->transform.position) * Matx4f::scaling(this->transform.scale);
    }
    return model;
}