// GPU picking system: renders entity IDs into a PickingTexture each frame and reads back the hit
// entity on request.
#pragma once
#include "Platform/CoreUtils/Math/matx.hpp"
#include "Platform/Graphics/PickingTexture.hpp"
#include "Platform/Graphics/Shader.hpp"
#include "System.hpp"
#include <memory>

class Camera;

class PickingSystem : public System {
public:
    PickingSystem();
    ~PickingSystem() override                      = default;
    PickingSystem(PickingSystem const&)            = delete;
    PickingSystem& operator=(PickingSystem const&) = delete;
    PickingSystem(PickingSystem&&)                 = delete;
    PickingSystem& operator=(PickingSystem&&)      = delete;

    void OnAttach(Scene* scene) override;
    void OnUpdate(float deltaTime) override;

    void RequestPickingPass(uint32_t screenX, uint32_t screenY);
    [[nodiscard]] PickingResult const& GetLastResult() const { return m_LastResult; }

    void OnWindowResize(uint32_t width, uint32_t height);
    void SetGlobalTransform(Matx4f const& global) { m_GlobalTransform = global; }

private:
    void ExecutePickingPass();
    void RenderPickingPass(Camera const& camera);

    std::unique_ptr<PickingTexture> m_PickingTexture;
    std::unique_ptr<Shader> m_PickingShader;
    PickingResult m_LastResult;

    Matx4f m_GlobalTransform  = Matx4f::identity();
    bool m_PickingRequested   = false;
    uint32_t m_PickX          = 0;
    uint32_t m_PickY          = 0;
    uint32_t m_ViewportWidth  = 1280;
    uint32_t m_ViewportHeight = 720;
};
