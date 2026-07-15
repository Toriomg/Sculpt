// Spawns scene entities from built-in primitives or imported asset files.
#pragma once
#include <memory>
#include <string>

class Scene;
class Shader;

enum class PrimitiveType { Cube, Sphere, Pyramid, Torus, Dodecahedron, Icosahedron };

class EntityFactory {
public:
    explicit EntityFactory(Scene* scene);

    void SpawnPrimitive(PrimitiveType type);
    void SpawnFromFile(const std::string& path);

private:
    Scene* m_Scene;
    std::shared_ptr<Shader> m_DefaultShader;
};
