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
    Matx4f Transform = Matx4f::identity();

    TransformComponent() = default;
    TransformComponent(const TransformComponent&) = default;
    TransformComponent& operator=(const TransformComponent&) = default;
    TransformComponent(const Matx4f& transform)
        : Transform(transform) {
    }
    Vec3 GetTranslation() const {
        return Vec3(Transform.m[0][3], Transform.m[1][3], Transform.m[2][3]);
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