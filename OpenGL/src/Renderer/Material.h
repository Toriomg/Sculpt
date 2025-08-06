#pragma once

#include "Platform/Graphics/Texture.h"
#include "Platform/Graphics/Shader.h"

#include "glhead.h"
class Material {
public:
    Material(std::shared_ptr<Shader> shader) : m_Shader(shader) {}

    void SetTexture(std::shared_ptr<Texture> texture) { m_Texture = texture; }

    const std::shared_ptr<Shader>& GetShader() const { return m_Shader; }
    const std::shared_ptr<Texture>& GetTexture() const { return m_Texture; }

private:
    std::shared_ptr<Shader> m_Shader;
    std::shared_ptr<Texture> m_Texture; // For simplicity, one texture for now
}; 