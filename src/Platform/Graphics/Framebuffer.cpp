#include "Framebuffer.hpp"
#include <GL/glew.h>
#include <cassert>  // For assert

Framebuffer::Framebuffer(uint32_t width, uint32_t height) : m_Width(width), m_Height(height) {
    // Invalidate() does all the creation work.
    Invalidate();
}

Framebuffer::~Framebuffer() {
    // Clean up all the GPU resources when this object is destroyed.
    glDeleteFramebuffers(1, &m_RendererID);
    glDeleteTextures(1, &m_ColorAttachmentID);
    glDeleteTextures(1, &m_DepthAttachmentID);
}

void Framebuffer::Invalidate() {
    // If we already have a framebuffer, delete the old resources first.
    // This is important for the Resize() functionality.
    if (m_RendererID != 0u) {
        glDeleteFramebuffers(1, &m_RendererID);
        glDeleteTextures(1, &m_ColorAttachmentID);
        glDeleteTextures(1, &m_DepthAttachmentID);
    }

    // --- Create the Framebuffer Object (FBO) ---
    glGenFramebuffers(1, &m_RendererID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

    // --- Create the Color Attachment (Texture) ---
    glGenTextures(1, &m_ColorAttachmentID);
    glBindTexture(GL_TEXTURE_2D, m_ColorAttachmentID);
    // Create an empty texture with the specified dimensions.
    // GL_RGBA8 is a common format for 8 bits per channel (Red, Green, Blue, Alpha).
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    // Set texture filtering parameters. GL_LINEAR provides smooth scaling.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Attach the texture to the framebuffer's color attachment point.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachmentID,
                           0);

    // --- Create the Depth/Stencil Attachment (Texture) ---
    glGenTextures(1, &m_DepthAttachmentID);
    glBindTexture(GL_TEXTURE_2D, m_DepthAttachmentID);
    // We create a texture with a combined depth and stencil format.
    // GL_DEPTH24_STENCIL8 is a common and well-supported format.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Width, m_Height, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, nullptr);
    // Attach the depth/stencil texture to the framebuffer.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           m_DepthAttachmentID, 0);

    // --- Final Check ---
    // After attaching everything, check if the framebuffer is "complete".
    // If it's not, something went wrong (e.g., GPU ran out of memory, formats are mismatched).
    // Using an assert is great for development as it will crash the program and point you here.
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE &&
           "Framebuffer is incomplete!");

    // Unbind the framebuffer so we don't accidentally render to it.
    // This returns rendering control to the default screen framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Resize(uint32_t width, uint32_t height) {
    // Don't resize if dimensions are invalid or the same as before.
    if (width == 0 || height == 0 || (m_Width == width && m_Height == height)) { return; }

    m_Width  = width;
    m_Height = height;
    // Recreate the entire framebuffer with the new dimensions.
    Invalidate();
}

void Framebuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
    // When we bind our FBO, we should also set the viewport to match its size.
    glViewport(0, 0, m_Width, m_Height);
}

void Framebuffer::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
