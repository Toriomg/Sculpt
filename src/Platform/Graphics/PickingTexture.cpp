#include "PickingTexture.hpp"
#include "Platform/CoreUtils/Log.hpp"
#include <GL/glew.h>

PickingTexture::PickingTexture(uint32_t width, uint32_t height)
    : m_Width(width), m_Height(height) {
    Invalidate();
}

PickingTexture::~PickingTexture() {
    if (m_FramebufferID) {
        glDeleteFramebuffers(1, &m_FramebufferID);
    }
    if (m_IDTextureID) {
        glDeleteTextures(1, &m_IDTextureID);
    }
    if (m_WorldPosTextureID) {
        glDeleteTextures(1, &m_WorldPosTextureID);
    }
    if (m_DepthTextureID) {
        glDeleteTextures(1, &m_DepthTextureID);
    }
}

void PickingTexture::Invalidate() {
    if (m_FramebufferID) {
        glDeleteFramebuffers(1, &m_FramebufferID);
        m_FramebufferID = 0;
    }
    if (m_IDTextureID) {
        glDeleteTextures(1, &m_IDTextureID);
        m_IDTextureID = 0;
    }
    if (m_WorldPosTextureID) {
        glDeleteTextures(1, &m_WorldPosTextureID);
        m_WorldPosTextureID = 0;
    }
    if (m_DepthTextureID) {
        glDeleteTextures(1, &m_DepthTextureID);
        m_DepthTextureID = 0;
    }

    glGenFramebuffers(1, &m_FramebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

    // Attachment 0: entity ID (red channel = entityID+1, green channel = primitiveID).
    // GL_NEAREST is mandatory — linear filtering would blend adjacent entity IDs producing garbage.
    glGenTextures(1, &m_IDTextureID);
    glBindTexture(GL_TEXTURE_2D, m_IDTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32UI, m_Width, m_Height, 0, GL_RG_INTEGER, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_IDTextureID, 0);

    // Attachment 1: world-space position as RGBA32F (xyz = position, w unused).
    // 32-bit float precision is required for accurate world coordinates at large scene scales.
    glGenTextures(1, &m_WorldPosTextureID);
    glBindTexture(GL_TEXTURE_2D, m_WorldPosTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_WorldPosTextureID, 0);

    glGenTextures(1, &m_DepthTextureID);
    glBindTexture(GL_TEXTURE_2D, m_DepthTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTextureID, 0);

    uint32_t attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        // Frame buffer is not complete
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PickingTexture::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
    glViewport(0, 0, m_Width, m_Height);
}

void PickingTexture::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PickingTexture::Resize(uint32_t width, uint32_t height) {
    m_Width = width;
    m_Height = height;
    Invalidate();
}

PickingResult PickingTexture::ReadPixel(uint32_t x, uint32_t y) const {
    PickingResult result;

    // OpenGL framebuffer origin is bottom-left; screen cursor origin is top-left.
    // -1 because y=0 must map to m_Height-1 (not m_Height which is out of bounds).
    uint32_t yGL = (y < m_Height) ? (m_Height - 1u - y) : 0u;

    CORE_LOG_TRACE("ReadPixel: viewport({},{}) -> gl({},{}) | FBO={}x{}",
        x, y, x, yGL, m_Width, m_Height);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FramebufferID);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    uint32_t idData[2] = { 0, 0 };
    glReadPixels(x, yGL, 1, 1, GL_RG_INTEGER, GL_UNSIGNED_INT, idData);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    float worldPos[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glReadPixels(x, yGL, 1, 1, GL_RGBA, GL_FLOAT, worldPos);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    CORE_LOG_TRACE("ReadPixel result: objectID={} primitiveID={}", idData[0], idData[1]);

    // id 0 means the pixel was cleared (no entity). Valid IDs were stored as entityID+1 by PickingSystem.
    if (idData[0] == 0u) {
        result.ObjectID = 0;
        result.Valid = false;
    } else {
        result.ObjectID = idData[0];
        result.Valid = true;
    }
    result.PrimitiveID = idData[1];
    result.WorldPosition = Vec3(worldPos[0], worldPos[1], worldPos[2]);

    return result;
}
