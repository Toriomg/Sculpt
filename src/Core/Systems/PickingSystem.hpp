// GPU picking system: renders entity IDs into a PickingTexture each frame and reads back the hit entity on request.
#pragma once
#include "System.hpp"
#include "Platform/Graphics/PickingTexture.hpp"
#include "Platform/Graphics/Shader.hpp"
#include "Renderer/Camera.hpp"
#include <memory>

class PickingSystem : public System {
public:
    PickingSystem();
    ~PickingSystem() override = default;

    void OnAttach(Scene* scene) override;
    void OnUpdate(float deltaTime) override;

    void RequestPickingPass(uint32_t screenX, uint32_t screenY);
    const PickingResult& GetLastResult() const { return m_LastResult; }

    void OnWindowResize(uint32_t width, uint32_t height);

private:
    void ExecutePickingPass();
    void RenderPickingPass(const Camera& camera);

    std::unique_ptr<PickingTexture> m_PickingTexture;
    std::unique_ptr<Shader> m_PickingShader;
    PickingResult m_LastResult;

    bool m_PickingRequested = false;
    uint32_t m_PickX = 0;
    uint32_t m_PickY = 0;
    uint32_t m_ViewportWidth = 1280;
    uint32_t m_ViewportHeight = 720;
};
