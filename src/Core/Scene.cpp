#include "Scene.hpp"
#include "Components/Component.hpp"
//Systems
#include "Systems/PickingSystem.hpp"
#include "Systems/SelectionSystem.hpp"
#include "Systems/RenderingSystem.hpp"

Scene::Scene()
{
	// Initialize systems to the scene
    m_Systems.emplace_back(std::make_unique<PickingSystem>());
    m_Systems.emplace_back(std::make_unique<SelectionSystem>());
    m_Systems.emplace_back(std::make_unique<RenderingSystem>());

    //Attach systems to the scene
    for (const auto& system : m_Systems) {
        system->OnAttach(this);
    }
}

Scene::~Scene() {}

Entity Scene::CreateGameObject(const std::string& name)
{
    Entity entity = m_Registry.create();
    // Every game object should have a transform by default
    AddComponent<TransformComponent>(entity);
    // Add a name component for easier identification in the UI
    AddComponent<NameComponent>(entity, name);
	LOG_INFO("Created GameObject with ID: {0} and Name: {1}", (uint32_t)entity, name);
    return entity;
}

void Scene::OnUpdate(float deltaTime)
{
    // Update all systems in the scene
    for (const auto& system : m_Systems)
    {
        system->OnUpdate(deltaTime);
    }
}