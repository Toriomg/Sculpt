#include "Editor/EntityFactory.hpp"
#include "Core/Scene.hpp"
#include "Core/Components/Component.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Material.hpp"
#include "Platform/Graphics/Shader.hpp"
#include "AssetManager/AssetManager.hpp"

EntityFactory::EntityFactory(Scene* scene)
    : m_Scene(scene)
    , m_DefaultShader(std::make_shared<Shader>("res/shaders/modelmesh.shader")) {}

namespace {
    std::string NameOf(PrimitiveType type) {
        switch (type) {
            case PrimitiveType::Cube:         return "Cube";
            case PrimitiveType::Sphere:       return "Sphere";
            case PrimitiveType::Pyramid:      return "Pyramid";
            case PrimitiveType::Torus:        return "Torus";
            case PrimitiveType::Dodecahedron: return "Dodecahedron";
            case PrimitiveType::Icosahedron:  return "Icosahedron";
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
        }
        return nullptr;
    }
}

void EntityFactory::SpawnPrimitive(PrimitiveType type) {
    auto mesh     = MeshOf(type);
    auto material = std::make_shared<Material>(m_DefaultShader);
    Entity entity = m_Scene->CreateGameObject(NameOf(type));
    m_Scene->AddComponent<MeshComponent>(entity, mesh, material);
    m_Scene->AddComponent<SelectionComponent>(entity);
}

void EntityFactory::SpawnFromFile(const std::string& path) {
    std::string name = path.substr(path.find_last_of("/\\") + 1);
    Entity entity = m_Scene->CreateGameObject(name);
    m_Scene->AddComponent<SelectionComponent>(entity);

    // Capture raw pointers matching the existing pattern — both live for the application lifetime.
    Scene* scene  = m_Scene;
    auto shader   = m_DefaultShader;
    AssetManager::LoadAsync(path, [scene, entity, shader](AssetHandle handle) {
        if (auto mesh = AssetManager::GetAs<Mesh>(handle)) {
            auto material = std::make_shared<Material>(shader);
            scene->AddComponent<MeshComponent>(entity, mesh, material);
        }
    });
}
