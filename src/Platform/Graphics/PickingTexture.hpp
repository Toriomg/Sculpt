#pragma once
#include <cstdint>
#include "Platform/CoreUtils/Math/vec3.hpp"
#include "Core/Entity.hpp"

struct PickingResult {
    Entity SelectedEntity = entt::null;
    uint32_t PrimitiveID = 0xFFFFFFFF;
    Vec3 WorldPosition = Vec3(0.0f, 0.0f, 0.0f);
    bool Valid = false;
};

class PickingTexture {
public:
    PickingTexture(uint32_t width, uint32_t height);
    ~PickingTexture();

    void Bind();
    void Unbind();
    void Resize(uint32_t width, uint32_t height);

    PickingResult ReadPixel(uint32_t x, uint32_t y) const;

private:
    void Invalidate();

    uint32_t m_FramebufferID = 0;
    uint32_t m_IDTextureID = 0;
    uint32_t m_WorldPosTextureID = 0;
    uint32_t m_DepthTextureID = 0;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
};
