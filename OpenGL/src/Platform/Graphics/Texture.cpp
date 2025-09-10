#include "Texture.h"

Texture::Texture(const TextureSpecification& specification, const void* data)
	: m_RendererID(0), m_Specification(specification)
{
	GLenum internalFormat = 0, dataFormat = 0;
	if (m_Specification.channels == 4)
	{
		internalFormat = GL_RGBA8;
		dataFormat = GL_RGBA;
	}
	else if (m_Specification.channels == 3)
	{
		internalFormat = GL_RGB8;
		dataFormat = GL_RGB;
	}
	else
	{
		CORE_LOG_ERROR("Unsupported number of texture channels: {0}", m_Specification.channels);
	}

	GLCall(glGenTextures(1, &m_RendererID));
	CORE_LOG_INFO("Generated Texture ID: {0}", m_RendererID);
	if (m_RendererID == 0) {
		CORE_LOG_ERROR("Failed to generate texture ID!");
	}
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));

	// Set texture filtering and wrapping parameters
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	// Upload the texture data to the GPU
	if (internalFormat != 0 && dataFormat != 0)
	{
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0, dataFormat, GL_UNSIGNED_BYTE, data));
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	}

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::~Texture() {
	GLCall(glDeleteTextures(1, &m_RendererID)); // Delete the texture from OpenGL
}

void Texture::Bind(unsigned int slot) const {
	GLCall(glActiveTexture(GL_TEXTURE0 + slot)); // Activate the texture unit
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID)); // Bind the texture to the specified slot
}

void Texture::Unbind() const {
	GLCall(glBindTexture(GL_TEXTURE_2D, 0)); // Unbind the texture
}