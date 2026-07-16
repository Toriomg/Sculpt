// Spawns scene entities from built-in primitives or imported asset files.
#pragma once
#include <expected>
#include <memory>
#include <string>

class Scene;
class Shader;

enum class PrimitiveType { Cube, Sphere, Pyramid, Torus, Dodecahedron, Icosahedron, Arrow, Cone };

class EntityFactory {
public:
    explicit EntityFactory(Scene* scene);

    void SpawnPrimitive(PrimitiveType type);

    // Returns an error string if the file does not exist; async load failures are logged.
    std::expected<void, std::string> SpawnFromFile(const std::string& path);

private:
    Scene* m_Scene;
    std::shared_ptr<Shader> m_DefaultShader;
};
