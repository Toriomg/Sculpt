// ECS system: iterates all entities with a MeshComponent and submits them to Renderer each frame.
#pragma once
#include "System.hpp"
#include "Platform/CoreUtils/Math/matx.hpp"

class Renderer;

class RenderingSystem : public System
{
public:
    RenderingSystem()  = default;
    ~RenderingSystem() = default;

    virtual void OnUpdate(float deltaTime) override;
    void SetGlobalTransform(const Matx4f& global) { m_GlobalTransform = global; }

private:
    Matx4f m_GlobalTransform = Matx4f::identity();
};