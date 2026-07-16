// All ECS component structs (Name, Transform, Mesh, Camera, Selection); included wherever entities are queried or created.
#pragma once
#include <memory>
#include <string>
#include "Renderer/Camera.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"

class Mesh;
class Material;

// Base component struct. All components will "inherit" from this conceptually.
// In practice, they just need to be simple data structs.
struct Component {};

struct NameComponent : public Component {
    std::string Name;

    NameComponent() = default;
    NameComponent(const NameComponent&) = default;
    NameComponent& operator=(const NameComponent&) = default;
    NameComponent(const std::string& name)
        : Name(name) {
    }
};

struct TransformComponent : public Component
{
    Vec3 Translation  = {0.0f, 0.0f, 0.0f};
    Vec3 EulerDegrees = {0.0f, 0.0f, 0.0f};
    Vec3 Scale        = {1.0f, 1.0f, 1.0f};

    TransformComponent() = default;
    TransformComponent(const TransformComponent&) = default;
    TransformComponent& operator=(const TransformComponent&) = default;

    Matx4f GetMatrix() const {
        return Matx4f::translation(Translation)
             * Matx4f::rotation(EulerDegrees.x, EulerDegrees.y, EulerDegrees.z)
             * Matx4f::scaling(Scale);
    }
};

struct MeshComponent : public Component
{
    std::shared_ptr<Mesh> MeshAsset;
    std::shared_ptr<Material> MaterialAsset;
    bool Wireframe = false;

    MeshComponent() = default;
    MeshComponent(const MeshComponent&) = default;
    MeshComponent& operator=(const MeshComponent&) = default;
    MeshComponent(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
        : MeshAsset(mesh), MaterialAsset(material) {
    }
};

struct CameraComponent : public Component
{
    Camera SceneCamera;
    bool IsPrimary = true;

    CameraComponent() = default;
    CameraComponent(const CameraComponent&) = default;
    CameraComponent& operator=(const CameraComponent&) = default;
};

struct SelectionComponent : public Component
{
    uint32_t SelectionGroup = 0;
    uint32_t CustomData = 0;
    bool AllowsOutline = true;

    SelectionComponent() = default;
    SelectionComponent(const SelectionComponent&) = default;
    SelectionComponent& operator=(const SelectionComponent&) = default;
    SelectionComponent(uint32_t group)
        : SelectionGroup(group) {
    }
};