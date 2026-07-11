#include "PickingSystem.hpp"
#include "Core/Scene.hpp"
#include "Core/Components/Component.hpp"
#include "Platform/Graphics/Shader.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Camera.hpp"
#include <GL/glew.h>

PickingSystem::PickingSystem()
    : m_PickingTexture(std::make_unique<PickingTexture>(m_ViewportWidth, m_ViewportHeight)),
      m_PickingShader(std::make_unique<Shader>("res/shaders/Picking.shader")) {
}

void PickingSystem::OnAttach(Scene* scene) {
    System::OnAttach(scene);
}

void PickingSystem::OnUpdate(float deltaTime) {
    if (!m_PickingRequested) {
        return;
    }

    ExecutePickingPass();
    m_PickingRequested = false;
}

void PickingSystem::RequestPickingPass(uint32_t screenX, uint32_t screenY) {
    m_PickingRequested = true;
    m_PickX = screenX;
    m_PickY = screenY;
}

void PickingSystem::ExecutePickingPass() {
    if (!m_Scene) {
        return;
    }

    Camera* camera = nullptr;
    auto cameraView = m_Scene->GetAllEntitiesWith<CameraComponent>();
    for (auto entity : cameraView) {
        auto& cam = cameraView.get<CameraComponent>(entity);
        if (cam.IsPrimary) {
            camera = &cam.SceneCamera;
            break;
        }
    }

    if (!camera) {
        return;
    }

    uint32_t cameraWidth = static_cast<uint32_t>(camera->GetViewportWidth());
    uint32_t cameraHeight = static_cast<uint32_t>(camera->GetViewportHeight());
    if (cameraWidth != m_ViewportWidth || cameraHeight != m_ViewportHeight) {
        OnWindowResize(cameraWidth, cameraHeight);
    }

    RenderPickingPass(*camera);
    m_LastResult = m_PickingTexture->ReadPixel(m_PickX, m_PickY);
}

void PickingSystem::RenderPickingPass(const Camera& camera) {
    m_PickingTexture->Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    m_PickingShader->Bind();
    m_PickingShader->SetUniformMat4f("u_ViewProjection", camera.GetViewProjectionMatrix());

    auto group = m_Scene->GetAllEntitiesWith<TransformComponent, MeshComponent, SelectionComponent>();

    for (auto entity : group) {
        auto& transform = group.get<TransformComponent>(entity);
        auto& mesh = group.get<MeshComponent>(entity);

        if (!mesh.MeshAsset) {
            continue;
        }

        uint32_t entityID = static_cast<uint32_t>(entity) + 1u;
        m_PickingShader->SetUniform1ui("u_ObjectID", entityID);
        m_PickingShader->SetUniformMat4f("u_Model", transform.Transform);

        auto va = mesh.MeshAsset->GetVertexArray();
        auto ib = mesh.MeshAsset->GetIndexBuffer();

        va->Bind();
        ib->Bind();
        glDrawElements(GL_TRIANGLES, ib->GetCount(), GL_UNSIGNED_INT, nullptr);
    }

    m_PickingTexture->Unbind();
    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
}

void PickingSystem::OnWindowResize(uint32_t width, uint32_t height) {
    m_ViewportWidth = width;
    m_ViewportHeight = height;
    m_PickingTexture->Resize(width, height);
}
