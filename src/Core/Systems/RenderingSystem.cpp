#include "RenderingSystem.hpp"
#include "SelectionSystem.hpp"
#include "Core/Scene.hpp"
#include "Core/Components/Component.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Camera.hpp"
#include "Platform/Graphics/Shader.hpp"


void RenderingSystem::OnUpdate(float deltaTime)
{
    // Safety check: ensure we have a scene to work with.
    if (!m_Scene) return;

    // 1. Find the primary camera in the scene.
    Camera* mainCamera = nullptr;
    Matx4f cameraTransform;
    {
        // Get all entities that have both a Transform and a Camera component.
        auto view = m_Scene->GetAllEntitiesWith<TransformComponent, CameraComponent>();
        for (auto entity : view)
        {
            auto& camComp = view.get<CameraComponent>(entity);
            if (camComp.IsPrimary)
            {
                mainCamera = &camComp.SceneCamera;
                break; // Found the primary camera, no need to look further.
            }
        }
    }

    // 2. If we found a camera, begin the rendering process.
    if (mainCamera)
    {


        Renderer::BeginScene(mainCamera->GetViewProjectionMatrix());

        auto selectionSystem = m_Scene->GetSystem<SelectionSystem>();
        const SelectionContext* selectionContext = nullptr;
        if (selectionSystem) {
            selectionContext = &selectionSystem->GetSelectionContext();
        }

        // 3. Find all entities that are renderable and submit them.
        auto group = m_Scene->GetAllEntitiesWith<TransformComponent, MeshComponent>();
        for (auto entity : group)
        {
            auto& transform = group.get<TransformComponent>(entity).Transform;
            auto& meshComp = group.get<MeshComponent>(entity);

            if (!meshComp.MeshAsset || !meshComp.MaterialAsset) {
                LOG_WARN("Entity {0} is missing a Mesh or Material!", (uint32_t)entity);
                continue;
            }

            meshComp.MaterialAsset->GetShader()->Bind();
            meshComp.MaterialAsset->GetShader()->SetUniform3f("u_cameraPos",
                mainCamera->GetPosition().x,
                mainCamera->GetPosition().y,
                mainCamera->GetPosition().z);

            bool isSelected = selectionContext && selectionContext->IsEntitySelected(entity);
            meshComp.MaterialAsset->GetShader()->SetUniform1i("u_IsSelected", isSelected ? 1 : 0);
            if (isSelected) {
                meshComp.MaterialAsset->GetShader()->SetUniform4f("u_HighlightColor", 1.0f, 1.0f, 0.0f, 1.0f);
            }

            Renderer::Submit(meshComp.MeshAsset, meshComp.MaterialAsset, transform);
        }
        // Tell the Renderer we are done with this frame.
        Renderer::EndScene();
    }
}