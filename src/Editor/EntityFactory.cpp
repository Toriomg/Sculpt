#include "Editor/EntityFactory.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "Platform/CoreUtils/Log.hpp"
#include "Platform/Graphics/Shader.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Mesh.hpp"
#include <filesystem>
#include <format>

EntityFactory::EntityFactory(Scene* scene)
    : m_Scene(scene), m_DefaultShader(std::make_shared<Shader>("res/shaders/modelmesh.shader")) { }

namespace {

    std::string NameOf(PrimitiveType type) {
        switch (type) {
            case PrimitiveType::Cube:         return "Cube";
            case PrimitiveType::Sphere:       return "Sphere";
            case PrimitiveType::Pyramid:      return "Pyramid";
            case PrimitiveType::Torus:        return "Torus";
            case PrimitiveType::Dodecahedron: return "Dodecahedron";
            case PrimitiveType::Icosahedron:  return "Icosahedron";
            case PrimitiveType::Arrow:        return "Arrow";
            case PrimitiveType::Cone:         return "Cone";
        }
        return "Object";
    }

    std::shared_ptr<Mesh> MeshOf(PrimitiveType type) {
        switch (type) {
            case PrimitiveType::Cube:         return Mesh::CreateCube(1.0f);
            case PrimitiveType::Sphere:       return Mesh::CreateSphere(1.0f, 32, 32);
            case PrimitiveType::Pyramid:      return Mesh::CreatePyramid(1.0f);
            case PrimitiveType::Torus:        return Mesh::CreateTorus(1.0f, 0.5f, 32, 32);
            case PrimitiveType::Dodecahedron: return Mesh::CreateDodecahedron(1.0f);
            case PrimitiveType::Icosahedron:  return Mesh::CreateIcosahedron(1.0f);
            case PrimitiveType::Arrow:        return Mesh::CreateArrow();
            case PrimitiveType::Cone:         return Mesh::CreateCone();
        }
        return nullptr;
    }

}  // namespace

void EntityFactory::SpawnPrimitive(PrimitiveType type) {
    auto mesh     = MeshOf(type);
    auto material = std::make_shared<Material>(m_DefaultShader);
    Entity const entity = m_Scene->CreateGameObject(NameOf(type));
    m_Scene->AddComponent<MeshComponent>(entity, mesh, material);
    m_Scene->AddComponent<SelectionComponent>(entity);
}

std::expected<void, std::string> EntityFactory::SpawnFromFile(std::string const& path) {
    if (!std::filesystem::exists(path)) {
        return std::unexpected(std::format("File not found: {}", path));
}

    std::string const name = path.substr(path.find_last_of("/\\") + 1);
    Entity const entity    = m_Scene->CreateGameObject(name);
    m_Scene->AddComponent<SelectionComponent>(entity);

    Scene* scene = m_Scene;
    auto* selSys = m_Scene->GetSystem<SelectionSystem>();
    auto shader  = m_DefaultShader;
    AssetManager::LoadAsync(path, [scene, entity, shader, selSys](AssetHandle handle) {
        auto mesh = AssetManager::GetAs<Mesh>(handle);
        if (!mesh) {
            CORE_LOG_ERROR("Mesh load failed for async asset; removing placeholder entity.");
            if (selSys) { selSys->GetSelectionContext().Deselect(entity);
}
            scene->DestroyEntity(entity);
            return;
        }
        auto material = std::make_shared<Material>(shader);
        scene->AddComponent<MeshComponent>(entity, mesh, material);
    });
    return {};
}
