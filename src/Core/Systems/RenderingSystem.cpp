#include "RenderingSystem.hpp"
#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"
#include "Core/glhead.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Renderer.hpp"
#include "SelectionSystem.hpp"

void RenderingSystem::OnUpdate(float deltaTime) {
    // Safety check: ensure we have a scene to work with.
    if (!m_Scene) return;

    // 1. Find the primary camera in the scene.
    Camera* mainCamera = nullptr;
    Matx4f cameraTransform;
    {
        // Get all entities that have both a Transform and a Camera component.
        auto view = m_Scene->GetAllEntitiesWith<TransformComponent, CameraComponent>();
        for (auto entity : view) {
            auto& camComp = view.get<CameraComponent>(entity);
            if (camComp.IsPrimary) {
                mainCamera = &camComp.SceneCamera;
                break;  // Found the primary camera, no need to look further.
            }
        }
    }

    // 2. If we found a camera, begin the rendering process.
    if (mainCamera) {
        Renderer::BeginScene(mainCamera->GetViewProjectionMatrix());

        auto selectionSystem                     = m_Scene->GetSystem<SelectionSystem>();
        SelectionContext const* selectionContext = nullptr;
        if (selectionSystem) { selectionContext = &selectionSystem->GetSelectionContext(); }

        // 3. Find all entities that are renderable and submit them.
        auto group = m_Scene->GetAllEntitiesWith<TransformComponent, MeshComponent>();
        for (auto entity : group) {
            auto& tc       = group.get<TransformComponent>(entity);
            auto& meshComp = group.get<MeshComponent>(entity);

            if (!meshComp.MeshAsset || !meshComp.MaterialAsset) {
                LOG_WARN("Entity {0} is missing a Mesh or Material!", (uint32_t) entity);
                continue;
            }

            Matx4f worldTransform = m_GlobalTransform * tc.GetMatrix();

            if (Renderer::IsDebugSelectionModeEnabled()) {
                auto const& dbgShader = Renderer::GetDebugSelectionShader();
                dbgShader->Bind();
                bool pickable = m_Scene->HasComponent<SelectionComponent>(entity);
                dbgShader->SetUniform1i("u_IsPickable", pickable ? 1 : 0);
                Renderer::Submit(meshComp.MeshAsset, meshComp.MaterialAsset, worldTransform);
            } else {
                // These uniforms are selection-state dependent, so they must be set here before
                // Renderer::Submit, which re-binds the same shader and sets the remaining uniforms.
                meshComp.MaterialAsset->GetShader()->Bind();
                meshComp.MaterialAsset->GetShader()->SetUniform3f(
                    "u_cameraPos", mainCamera->GetPosition().x, mainCamera->GetPosition().y,
                    mainCamera->GetPosition().z);

                bool isSelected = selectionContext && selectionContext->IsEntitySelected(entity);

                if (isSelected) {
                    // Stencil write pass: mark pixels covered by this entity
                    glEnable(GL_STENCIL_TEST);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                    glStencilFunc(GL_ALWAYS, 1, 0xFF);
                    glStencilMask(0xFF);
                }

                if (meshComp.Wireframe)
                    Renderer::SubmitWireframe(meshComp.MeshAsset, worldTransform);
                else Renderer::Submit(meshComp.MeshAsset, meshComp.MaterialAsset, worldTransform);

                if (isSelected) {
                    // Outline pass: draw scaled mesh only where stencil == 0 (outside the entity)
                    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                    glStencilMask(0x00);
                    glDisable(GL_DEPTH_TEST);
                    Renderer::SubmitOutline(meshComp.MeshAsset, Vec4(1.0f, 0.5f, 0.0f, 1.0f),
                                            0.008f, worldTransform);
                    // Restore state
                    glEnable(GL_DEPTH_TEST);
                    glStencilMask(0xFF);
                    glDisable(GL_STENCIL_TEST);
                }
            }
        }
        // Tell the Renderer we are done with this frame.
        Renderer::EndScene();
    }
}
