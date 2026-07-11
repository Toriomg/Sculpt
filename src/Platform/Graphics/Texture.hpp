// OpenGL 2D texture implementing IAsset; wraps a GL texture ID and exposes Bind/Unbind by slot.
#pragma once
#include "Platform/CoreUtils/glewDbg.hpp"
#include "Core/glhead.hpp"
#include "AssetManager/IAsset.hpp"

struct TextureSpecification
{
	uint32_t Width = 1;
	uint32_t Height = 1;
	int channels = 4; // Default to RGBA
	// You can add more formats later, e.g., RGB, Float textures, etc.
	// GLenum Format = GL_RGBA8; 
};

class Texture : public IAsset
{
private:
	uint32_t m_RendererID;
	TextureSpecification m_Specification;
public:
	Texture(const TextureSpecification& specification, const void* data = nullptr);
	virtual ~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_Specification.Width; }
	inline int GetHeight() const { return m_Specification.Height; }
	uint32_t GetRendererID() const { return m_RendererID; }
};