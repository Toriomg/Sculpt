// All ECS component structs (Name, Transform, Mesh, Camera, Selection); included wherever entities
// are queried or created.
#pragma once
#include "Platform/CoreUtils/Math/maths.hpp"
#include "Renderer/Camera.hpp"
#include <memory>
#include <string>

class Mesh;
class Material;

struct NameComponent {
    std::string Name;

    NameComponent()                                = default;
    NameComponent(NameComponent const&)            = default;
    NameComponent& operator=(NameComponent const&) = default;
    NameComponent(std::string const& name) : Name(name) { }
};

struct TransformComponent {
    Vec3 Translation  = {0.0f, 0.0f, 0.0f};
    Vec3 EulerDegrees = {0.0f, 0.0f, 0.0f};
    Vec3 Scale        = {1.0f, 1.0f, 1.0f};

    TransformComponent()                                     = default;
    TransformComponent(TransformComponent const&)            = default;
    TransformComponent& operator=(TransformComponent const&) = default;

    Matx4f GetMatrix() const {
        return Matx4f::translation(Translation) *
               Matx4f::rotation(EulerDegrees.x, EulerDegrees.y, EulerDegrees.z) *
               Matx4f::scaling(Scale);
    }
};

struct MeshComponent {
    std::shared_ptr<Mesh> MeshAsset;
    std::shared_ptr<Material> MaterialAsset;
    bool Wireframe = false;

    MeshComponent()                                = default;
    MeshComponent(MeshComponent const&)            = default;
    MeshComponent& operator=(MeshComponent const&) = default;
    MeshComponent(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
        : MeshAsset(mesh), MaterialAsset(material) { }
};

struct CameraComponent {
    Camera SceneCamera;
    bool IsPrimary = true;

    CameraComponent()                                  = default;
    CameraComponent(CameraComponent const&)            = default;
    CameraComponent& operator=(CameraComponent const&) = default;
};

struct SelectionComponent {
    uint32_t SelectionGroup = 0;
    uint32_t CustomData     = 0;
    bool AllowsOutline      = true;

    SelectionComponent()                                     = default;
    SelectionComponent(SelectionComponent const&)            = default;
    SelectionComponent& operator=(SelectionComponent const&) = default;
    SelectionComponent(uint32_t group) : SelectionGroup(group) { }
};
