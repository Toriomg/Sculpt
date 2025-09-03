#include "RenderingSystem.h"
#include "Core/Scene.h"
#include "Core/Components/Component.h"
#include "Renderer/Renderer.h" // The system needs to talk to the Renderer API
#include "Renderer/Camera.h"   // To find the main camera


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


        Renderer::BeginScene(mainCamera->GetViewProjectionMatrix()); // Assuming the camera object itself has pos/rot

        // 3. Find all entities that are renderable and submit them.
        auto group = m_Scene->GetAllEntitiesWith<TransformComponent, MeshComponent>();
        unsigned int debugRenderEntities = 0;
        for (auto entity : group)
        {
            auto& transform = group.get<TransformComponent>(entity).Transform;
            auto& meshComp = group.get<MeshComponent>(entity);

            if (!meshComp.MeshAsset || !meshComp.MaterialAsset) {
                LOG_WARN("Entity {0} is missing a Mesh or Material!", (uint32_t)entity);
                continue; // Skip if mesh or material is missing.
            }

            meshComp.MaterialAsset->GetShader()->Bind();
            // Now set the camera position
            meshComp.MaterialAsset->GetShader()->SetUniform3f("u_cameraPos",
                mainCamera->GetPosition().x,
                mainCamera->GetPosition().y,
                mainCamera->GetPosition().z);
            // Submit this entity to the renderer to be drawn.
            Renderer::Submit(meshComp.MeshAsset, meshComp.MaterialAsset, transform);
            debugRenderEntities++;
        }
        //CORE_LOG_INFO("{0} Rendered entities", debugRenderEntities);
        // Tell the Renderer we are done with this frame.
        Renderer::EndScene();
    }
}