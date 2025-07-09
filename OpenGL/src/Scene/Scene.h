#pragma once

#include <vector>
#include <memory>
#include <string>
#include "GameObject.h"

class Scene {
public:
    Scene();
    ~Scene();

    GameObject* CreateGameObject(const std::string& name = "GameObject");

    void OnUpdate(float deltaTime);
    void OnRender(); // This will eventually be replaced by the main Renderer

private:
    std::vector<std::unique_ptr<GameObject>> m_GameObjects;
};