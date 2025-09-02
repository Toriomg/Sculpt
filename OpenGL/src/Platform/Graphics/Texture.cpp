#include "Texture.h"

Texture::Texture(const TextureSpecification& specification, const void* data)
	: m_RendererID(0), m_Specification(specification)
{
	glGenTextures(1, &m_RendererID);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);

	// Set texture filtering and wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Upload the texture data to the GPU
	// NOTE: For now we assume RGBA8 format. This can be extended using the specification.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Specification.Width, m_Specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
	glDeleteTextures(1, &m_RendererID); // Delete the texture from OpenGL
}

void Texture::Bind(unsigned int slot) const {
	glActiveTexture(GL_TEXTURE0 + slot); // Activate the texture unit
	glBindTexture(GL_TEXTURE_2D, m_RendererID); // Bind the texture to the specified slot
}

void Texture::Unbind() const {
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
}