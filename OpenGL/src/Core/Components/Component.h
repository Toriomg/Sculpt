#pragma once
#include <memory>
#include "Renderer/Mesh.h"
#include "Renderer/Material.h"
#include "Renderer/Camera.h"

// Base component struct. All components will "inherit" from this conceptually.
// In practice, they just need to be simple data structs.
struct Component {};

struct NameComponent : public Component {
    std::string Name;

    NameComponent() = default;

    // Copy constructor
    NameComponent(const NameComponent&) = default;

    // The crucial constructor that takes a string
    NameComponent(const std::string& name)
        : Name(name) {
    }
};

struct TransformComponent : public Component
{
    Matx4f Transform = Matx4f::identity();

    // Default constructors are fine
    TransformComponent() = default;
    TransformComponent(const TransformComponent&) = default;
    TransformComponent(const Matx4f& transform)
        : Transform(transform) {
    }
};

struct MeshComponent : public Component
{
    std::shared_ptr<Mesh> MeshAsset;
    std::shared_ptr<Material> MaterialAsset;

    MeshComponent() = default;
    MeshComponent(const MeshComponent&) = default;
    MeshComponent(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
        : MeshAsset(mesh), MaterialAsset(material) {
    }
};

struct CameraComponent : public Component
{
    // The component owns the actual Camera object
    Camera SceneCamera;
    bool IsPrimary = true; // Is this the main camera for the scene?

    CameraComponent() = default;
    CameraComponent(const CameraComponent&) = default;
};