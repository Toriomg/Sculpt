#pragma once
#include "Shader.h"
#include "Texture.h"

#include <memory>
#include <vector>

// Forward declarations
class Shader;
class Texture;

/*
    Represents the surface properties of a Mesh.
    It holds shared pointers to resources like Shaders and Textures,
    allowing multiple materials to reuse the same resources.
*/
class Material
{
public:
    // A material must have a shader.
    Material(const std::shared_ptr<Shader>& shader);
    ~Material() = default;

    // Add a texture to this material.
    void AddTexture(const std::shared_ptr<Texture>& texture);

    // Binds the material's shader and all of its textures for rendering.
    void Bind() const;
    void Unbind() const;

    // Public access for simplicity, allows direct manipulation if needed.
    std::shared_ptr<Shader> m_Shader;
    std::vector<std::shared_ptr<Texture>> m_Textures;

    
    Vec4 m_Color;
    float m_Shininess;
};