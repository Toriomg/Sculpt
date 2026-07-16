// ECS system: iterates all entities with a MeshComponent and submits them to Renderer each frame.
#pragma once
#include "Platform/CoreUtils/Math/matx.hpp"
#include "System.hpp"

class Renderer;

class RenderingSystem : public System {
public:
    RenderingSystem()  = default;
    ~RenderingSystem() = default;

    virtual void OnUpdate(float deltaTime) override;
    void SetGlobalTransform(Matx4f const& global) { m_GlobalTransform = global; }

private:
    Matx4f m_GlobalTransform = Matx4f::identity();
};
