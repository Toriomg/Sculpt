// OpenGL 2D texture wrapping a GL texture ID; exposes Bind/Unbind by slot.
#pragma once
#include "Platform/CoreUtils/AssetHandle.hpp"
#include <cstdint>

struct TextureSpecification {
    uint32_t Width  = 1;
    uint32_t Height = 1;
    int channels    = 4;  // Default to RGBA
};

class Texture {
public:
    AssetHandle Handle;

    Texture(TextureSpecification const& specification, void const* data = nullptr);
    ~Texture();
    Texture(Texture const&)            = delete;
    Texture& operator=(Texture const&) = delete;
    Texture(Texture&&)                 = default;
    Texture& operator=(Texture&&)      = default;

    void Bind(unsigned int slot = 0) const;
    static void Unbind();

    int GetWidth() const { return static_cast<int>(m_Specification.Width); }
    int GetHeight() const { return static_cast<int>(m_Specification.Height); }
    uint32_t GetRendererID() const { return m_RendererID; }

private:
    uint32_t m_RendererID;
    TextureSpecification m_Specification;
};
