// Abstract base for all scene systems; Scene calls OnAttach once and OnUpdate every frame, and
// holds a Scene* back-pointer.
#pragma once

// Forward-declare Scene to avoid circular dependencies
class Scene;

// Abstract base class for all systems
class System {
public:
    virtual ~System() = default;

    // Called when the system is first added to a scene
    virtual void OnAttach(Scene* scene) { m_Scene = scene; }

    // Called every frame to update the system's logic
    virtual void OnUpdate(float deltaTime) = 0;

protected:
    Scene* m_Scene = nullptr;
};
