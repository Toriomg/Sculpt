#include "Layer.h"
#include "Renderer/Renderer.h"
#include "Renderer/Camera.h"
#include "Renderer/Mesh.h"
#include "Renderer/Material.h"
#include <memory>

class ServiceLayer : public Layer {
public:
    ServiceLayer();
    virtual ~ServiceLayer() = default;

    virtual void OnAttach() override;
    virtual void OnUpdate(float deltaTime) override;
    //virtual void OnEvent(Event& event) override;

private:
    Camera m_EditorCamera;
    std::shared_ptr<Mesh> m_CubeMesh;
    std::shared_ptr<Material> m_CubeMaterial;
    Vec3 m_CubePosition = { 0.0f, 0.0f, 0.0f };
};