// Pairs a Shader with a texture AssetHandle; submitted alongside a Mesh to Renderer::Submit.
#pragma once

#include <memory>
#include "Platform/Graphics/Shader.hpp"
#include "Platform/CoreUtils/AssetHandle.hpp"
class Material {
public:
    Material(std::shared_ptr<Shader> shader) : m_Shader(shader) {}

    void SetTexture(AssetHandle textureHandle) { m_TextureHandle = textureHandle; }
    void SetWireframe(bool enabled) { m_Wireframe = enabled; }

    const std::shared_ptr<Shader>& GetShader() const { return m_Shader; }
    AssetHandle GetTextureHandle() const { return m_TextureHandle; }
    bool IsWireframe() const { return m_Wireframe; }

private:
    std::shared_ptr<Shader> m_Shader;
    AssetHandle m_TextureHandle;
    bool m_Wireframe = false;
}; 