#pragma once
#include "entt/entt.hpp"
#include "Entity.hpp"
#include "Systems/System.hpp"
#include <vector>
#include <memory>

class Scene
{
public:
    Scene();
    ~Scene();

    // The main update loop for the scene, called by your EditorLayer
    void OnUpdate(float deltaTime);

    // --- Entity & Component Management ---

    // Create a new entity (in your diagram, this is a GameObject)
    Entity CreateGameObject(const std::string& name = "GameObject");

    // Add a component to an entity
    template<typename T, typename... Args>
    T& AddComponent(Entity entity, Args&&... args) {
        return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
    }

    // Get a component from an entity
    template<typename T>
    T& GetComponent(Entity entity) {
        return m_Registry.get<T>(entity);
    }

    template<typename T, typename... Args>
    T& SetComponent(Entity entity, Args&&... args) {
        if (HasComponent<T>(entity)) {
            // Sobreescribimos el componente existente usando m_Registry.replace
            return m_Registry.replace<T>(entity, std::forward<Args>(args)...);
        }
        else {
            // Si no existe, lo creamos normalmente
            return AddComponent<T>(entity, std::forward<Args>(args)...);
        }
    }

    // Check if an entity has a component
    template<typename T>
    bool HasComponent(Entity entity) {
        return m_Registry.all_of<T>(entity);
    }

    // Remove a component from an entity
    template<typename T>
    void RemoveComponent(Entity entity) {
        m_Registry.remove<T>(entity);
    }

    // Get a view of all entities that have a specific set of components
    template<typename... Components>
    auto GetAllEntitiesWith() {
        return m_Registry.view<Components...>();
    }

    // Get a system by type
    template<typename T>
    T* GetSystem() {
        for (auto& system : m_Systems) {
            if (auto* castedSystem = dynamic_cast<T*>(system.get())) {
                return castedSystem;
            }
        }
        return nullptr;
    }

private:
    // The core ECS registry that stores all entities and components
    entt::registry m_Registry;

    // The scene owns all its systems
    std::vector<std::unique_ptr<System>> m_Systems;
};