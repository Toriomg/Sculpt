#include "Layer.h"
#include "Renderer/Camera.h"
#include "Renderer/Mesh.h"
#include "Renderer/Material.h"
#include <memory>

class RenderLayer : public Layer {
public:
    RenderLayer();
    virtual void OnUpdate(float deltaTime) override;
    virtual void OnEvent(Event& event) override;

private:
    Camera m_EditorCamera;
    std::shared_ptr<Mesh> m_CubeMesh;
    std::shared_ptr<Material> m_CubeMaterial;
    glm::vec3 m_CubePosition = { 0.0f, 0.0f, 0.0f };
};