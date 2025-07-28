#pragma once

#include <vector>
#include <memory>
#include <string>
#include "Components/Component.h"
#include "../Math/maths.h"

// Let's create a simple Transform struct for clarity
struct Transform
{
    Vec3 position = { 0.0f, 0.0f, 0.0f };
    Quaternion rotation = Quaternion::identity();
    Vec3 scale = { 1.0f, 1.0f, 1.0f };
};


class GameObject {
public:
    GameObject(const std::string& name = "GameObject");

    void OnUpdate(float deltaTime);
    void OnRender();

    // The magic function: Add a component of a specific type
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        // Create a new component
        std::unique_ptr<T> newComponent = std::make_unique<T>(std::forward<Args>(args)...);
        // Set its owner
        newComponent->SetOwner(this);
        // Call its attach method
        newComponent->OnAttach();
        // Add it to our list
        m_Components.push_back(std::move(newComponent));
        // Return a raw pointer for immediate use
        return static_cast<T*>(m_Components.back().get());
    }

    // The other magic function: Get a component of a specific type
    template<typename T>
    T* GetComponent() {
        for (const auto& component : m_Components) {
            // Use dynamic_cast to safely check if the component is of the requested type
            T* target = dynamic_cast<T*>(component.get());
            if (target) {
                return target;
            }
        }
        return nullptr; // Not found
    }

	void SetPickingID(int id) { m_PickingID = id; }

	int GetPickingID() const { return m_PickingID; }

    Matx4f GetTransformMatrix() const;

    std::string name;
    Transform transform;
    bool m_IsVisible = true;
private:
    std::vector<std::unique_ptr<Component>> m_Components;
	int m_PickingID = 0;
};