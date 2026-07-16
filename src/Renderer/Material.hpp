// Pairs a Shader with a texture AssetHandle; submitted alongside a Mesh to Renderer::Submit.
#pragma once

#include "Platform/CoreUtils/AssetHandle.hpp"
#include "Platform/Graphics/Shader.hpp"
#include <memory>
class Material {
public:
    Material(std::shared_ptr<Shader> shader) : m_Shader(shader) { }

    void SetTexture(AssetHandle textureHandle) { m_TextureHandle = textureHandle; }

    std::shared_ptr<Shader> const& GetShader() const { return m_Shader; }
    AssetHandle GetTextureHandle() const { return m_TextureHandle; }

private:
    std::shared_ptr<Shader> m_Shader;
    AssetHandle m_TextureHandle;
};
