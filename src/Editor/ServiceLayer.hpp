// Initialization layer: bootstraps Renderer and AssetManager on attach; currently holds temporary
// demo geometry.
#pragma once
#include "Platform/Layers/Layer.hpp"

class ServiceLayer : public Layer {
public:
    ServiceLayer();
    virtual ~ServiceLayer() = default;

    virtual void OnAttach() override;
    virtual void OnUpdate(float deltaTime) override;
};
