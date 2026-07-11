#pragma once
#include <cstdint>

class Framebuffer {
public:
    // Constructor creates the framebuffer with a specific size
    Framebuffer(uint32_t width, uint32_t height);
    // Destructor ensures all OpenGL objects are deleted
    ~Framebuffer();

    // Recreates the framebuffer and its attachments if the size changes
    void Resize(uint32_t width, uint32_t height);

    // Binds this framebuffer as the current render target
    void Bind();
    // Binds the default framebuffer (the screen) as the render target
    void Unbind();

    // Getter for the color attachment texture ID.
    // This is what you will render to the screen or a UI panel.
    uint32_t GetColorAttachmentRendererID() const { return m_ColorAttachmentID; }

private:
    // This private method holds all the logic for creating/recreating the framebuffer.
    // It's called by the constructor and the Resize() method.
    void Invalidate();

private:
    uint32_t m_RendererID = 0; // The ID of the framebuffer object itself

    // Attachments
    uint32_t m_ColorAttachmentID = 0; // The ID of the texture rendered color
    uint32_t m_DepthAttachmentID = 0; // The ID of the texture used for depth testing

    // Specifications
    uint32_t m_Width;
    uint32_t m_Height;
};