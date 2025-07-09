#include "Scene.h"

Scene::Scene() {}
Scene::~Scene() {}

GameObject* Scene::CreateGameObject(const std::string& name) {
	// Create a new GameObject with the given name and add it to the scene's list of GameObjects
    auto gameObject = std::make_unique<GameObject>(name);
    m_GameObjects.push_back(std::move(gameObject));
	// Return a pointer to the newly created GameObject
    return m_GameObjects.back().get();
}

void Scene::OnUpdate(float deltaTime) {
    for (const auto& go : m_GameObjects) {
        go->OnUpdate(deltaTime);
    }
}

void Scene::OnRender() {
    for (const auto& go : m_GameObjects) {
        go->OnRender();
    }
}