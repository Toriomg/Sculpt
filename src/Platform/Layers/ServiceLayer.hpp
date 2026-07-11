// Initialization layer: bootstraps Renderer and AssetManager on attach; currently holds temporary demo geometry.
#pragma once
#include "Layer.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Material.hpp"
#include "AssetManager/AssetManager.hpp"
#include <memory>

class ServiceLayer : public Layer {
public:
    ServiceLayer();
    virtual ~ServiceLayer() = default;

    virtual void OnAttach() override;
    virtual void OnUpdate(float deltaTime) override;
    //virtual void OnEvent(Event& event) override;

private:
    Camera m_EditorCamera;
    std::shared_ptr<Mesh> m_CubeMesh;
    std::shared_ptr<Material> m_CubeMaterial;
    Vec3 m_CubePosition = { 0.0f, 0.0f, 0.0f };
};