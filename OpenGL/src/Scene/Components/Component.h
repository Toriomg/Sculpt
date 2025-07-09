#pragma once

// Forward-declare GameObject to avoid circular dependencies
class GameObject;

class Component {
public:
    // Virtual destructor is essential for base classes
    virtual ~Component() = default;

    // Called when the component is first attached to a GameObject
    virtual void OnAttach() {}
    // Called once per frame
    virtual void OnUpdate(float deltaTime) {}
    // Called when rendering is needed (e.g., for gizmos or UI, not typically for meshes)
    virtual void OnRender() {}

    // Set the owner GameObject (called internally by GameObject)
    void SetOwner(GameObject* owner) { m_Owner = owner; }

protected:
    // A pointer to the GameObject this component is attached to.
    // This allows components to interact with their owner or other components.
    GameObject* m_Owner = nullptr;
};