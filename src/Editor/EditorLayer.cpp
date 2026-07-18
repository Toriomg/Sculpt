#include "Editor/EditorLayer.hpp"
#include "Core/Components/Component.hpp"
#include "Core/EditMesh/EditModeSystem.hpp"
#include "Core/Systems/HistorySystem.hpp"
#include "Core/Systems/PickingSystem.hpp"
#include "Core/Systems/RenderingSystem.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "Editor/EntityFactory.hpp"
#include "Editor/Gizmos/Gizmo.hpp"
#include "Editor/Gizmos/GizmoRenderer.hpp"
#include "Editor/Panels/InspectorPanel.hpp"
#include "Editor/Panels/MainMenuBar.hpp"
#include "Editor/Panels/OutlinerPanel.hpp"
#include "Editor/Panels/ScenePanel.hpp"
#include "Editor/Panels/ViewportPanel.hpp"
#include "Platform/Graphics/Framebuffer.hpp"
#include "Platform/Graphics/RenderCommand.hpp"
#include "Platform/System/Input/Input.hpp"
#include "Platform/System/Input/KeyCodes.hpp"
#include "Renderer/Renderer.hpp"
#include "imgui.h"

EditorLayer::EditorLayer(std::function<void()> onQuit)
    : Layer("EditorLayer"), m_CameraEntity(entt::null), m_OnQuit(std::move(onQuit)) { }

// Destructor defined here so forward-declared types (Framebuffer, panels) are fully visible.
EditorLayer::~EditorLayer() = default;

void EditorLayer::OnAttach() {
    CORE_LOG_INFO("EditorLayer attached!");

    m_ActiveScene = std::make_unique<Scene>();

    m_CameraEntity           = m_ActiveScene->CreateGameObject("Main Camera");
    auto& camComp            = m_ActiveScene->AddComponent<CameraComponent>(m_CameraEntity);
    auto& camTransform       = m_ActiveScene->GetComponent<TransformComponent>(m_CameraEntity);
    camTransform.Translation = Vec3(0.0f, 0.0f, 5.0f);
    camComp.SceneCamera.SetPosition({0.0f, 1.0f, 5.0f});

    m_EntityFactory  = std::make_unique<EntityFactory>(m_ActiveScene.get());
    m_EditModeSystem = std::make_unique<EditModeSystem>(m_ActiveScene.get(), camComp.SceneCamera);
    if (auto r = m_EntityFactory->SpawnFromFile("res/models/monkey.obj"); !r) {
        CORE_LOG_ERROR("Default scene asset missing: {}", r.error());
    }

    // Stores a raw pointer into the ECS component — safe while the entity lives.
    m_CameraController = std::make_unique<EditorCameraController>(&camComp.SceneCamera);
    m_Grid             = std::make_unique<InfGrid>();
    m_ViewportFBO      = std::make_unique<Framebuffer>(1470, 810);

    auto* selSys = m_ActiveScene->GetSystem<SelectionSystem>();

    m_GizmoRenderer = std::make_unique<GizmoRenderer>(*m_ActiveScene, selSys->GetSelectionContext(),
                                                      camComp.SceneCamera);
    m_GizmoRenderer->OnAttach();

    m_ViewportPanel = std::make_unique<ViewportPanel>(
        m_ViewportFBO.get(), [this](uint32_t w, uint32_t h) { OnViewportResize(w, h); });
    m_OutlinerPanel  = std::make_unique<OutlinerPanel>(m_ActiveScene.get(), selSys);
    m_InspectorPanel = std::make_unique<InspectorPanel>(
        m_ActiveScene.get(), &selSys->GetSelectionContext(), m_GizmoRenderer.get());
    m_InspectorPanel->SetEditModeSystem(m_EditModeSystem.get());
    m_ScenePanel  = std::make_unique<ScenePanel>(&camComp.SceneCamera);
    m_MainMenuBar = std::make_unique<MainMenuBar>(
        m_OnQuit, m_ActiveScene->GetSystem<HistorySystem>(), m_EntityFactory.get(),
        m_OutlinerPanel.get(), m_InspectorPanel.get(), m_ScenePanel.get(), m_ViewportPanel.get());
}

void EditorLayer::OnViewportResize(uint32_t width, uint32_t height) {
    m_ViewportFBO->Resize(width, height);
    auto& camComp = m_ActiveScene->GetComponent<CameraComponent>(m_CameraEntity);
    camComp.SceneCamera.SetViewportSize(static_cast<float>(width), static_cast<float>(height));
    RenderCommand::SetViewport(0, 0, width, height);
    auto* pickSys = m_ActiveScene->GetSystem<PickingSystem>();
    if (pickSys != nullptr) { pickSys->OnWindowResize(width, height); }
    if (m_GizmoRenderer) { m_GizmoRenderer->SetViewportSize(width, height); }
    if (m_EditModeSystem) { m_EditModeSystem->SetViewportSize(width, height); }
}

void EditorLayer::OnUpdate(float deltaTime) {
    auto& camComp      = m_ActiveScene->GetComponent<CameraComponent>(m_CameraEntity);
    auto& camTransform = m_ActiveScene->GetComponent<TransformComponent>(m_CameraEntity);

    // Camera movement is gated on viewport focus so ImGui text fields can receive keyboard input.
    if (!m_ViewportPanel || m_ViewportPanel->IsFocused()) {
        m_CameraController->OnUpdate(deltaTime);
    }

    // Sync TransformComponent to camera position driven by the controller.
    camTransform.Translation = camComp.SceneCamera.GetPosition();

    // Pass the ScenePanel's global transform to render and pick systems so they apply it
    // at draw time without baking it into entity TRS data.
    if (m_ScenePanel) {
        Matx4f const global = m_ScenePanel->GetGlobalTransform();
        if (auto* rs = m_ActiveScene->GetSystem<RenderingSystem>()) {
            rs->SetGlobalTransform(global);
        }
        if (auto* ps = m_ActiveScene->GetSystem<PickingSystem>()) {
            ps->SetGlobalTransform(global);
        }
        if (m_GizmoRenderer) { m_GizmoRenderer->SetGlobalTransform(global); }
        if (m_EditModeSystem) { m_EditModeSystem->SetGlobalTransform(global); }
    }

    // Render the scene into the viewport FBO.
    m_ViewportFBO->Bind();
    m_ActiveScene->OnUpdate(deltaTime);
    if (m_InEditMode && m_EditModeSystem) {
        Matx4f const global =
            m_ScenePanel ? m_ScenePanel->GetGlobalTransform() : Matx4f::identity();
        m_EditModeSystem->DrawOverlay(global);
    }
    m_Grid->Draw(camComp.SceneCamera.GetViewMatrix(), camComp.SceneCamera.GetProjectionMatrix(),
                 camComp.SceneCamera.GetPosition());
    if (!m_InEditMode) { m_GizmoRenderer->Draw(); }
    Framebuffer::Unbind();
}

void EditorLayer::OnImGuiRender() {
    m_MainMenuBar->OnImGuiRender();
    m_ViewportPanel->OnImGuiRender();
    m_OutlinerPanel->OnImGuiRender();
    m_InspectorPanel->OnImGuiRender();
    m_ScenePanel->OnImGuiRender();
}

void EditorLayer::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseButtonPressedEvent>(
        [this](MouseButtonPressedEvent& e) { return OnMouseButtonPressed(e); });
    dispatcher.Dispatch<MouseButtonReleasedEvent>(
        [this](MouseButtonReleasedEvent& e) { return OnMouseButtonReleased(e); });
    dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) { return OnKeyPressed(e); });
    dispatcher.Dispatch<KeyReleasedEvent>([](KeyReleasedEvent& e) { return OnKeyReleased(e); });
    dispatcher.Dispatch<MouseMovedEvent>([this](MouseMovedEvent& e) { return OnMouseMoved(e); });
    dispatcher.Dispatch<MouseScrolledEvent>(
        [this](MouseScrolledEvent& e) { return OnMouseScrolled(e); });
    dispatcher.Dispatch<WindowResizeEvent>([](WindowResizeEvent& e) { return OnWindowResize(e); });
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e) {
    if (e.GetMouseButton() != 0) { return false; }
    if (!m_ViewportPanel || !m_ViewportPanel->IsHovered()) { return false; }

    Vec2 mousePos    = Input::GetMousePosition();
    Vec2 viewportMin = m_ViewportPanel->GetViewportMin();
    Vec2 relPos      = mousePos - viewportMin;

    if (relPos.x < 0.0f || relPos.y < 0.0f) { return false; }

    bool const isShiftHeld = Input::IsKeyPressed(KeyCode::LeftShift);

    // In edit mode, clicks go to element selection via the picking system.
    if (m_InEditMode && m_EditModeSystem) {
        // A click while a grab is active confirms the extrude placement.
        if (m_EditModeSystem->IsGrabActive()) {
            m_EditModeSystem->ConfirmGrab();
            return true;
        }
        auto* pickSys = m_ActiveScene->GetSystem<PickingSystem>();
        if (pickSys == nullptr) { return false; }
        pickSys->RequestPickingPass(static_cast<uint32_t>(relPos.x),
                                    static_cast<uint32_t>(relPos.y));
        pickSys->OnUpdate(0.0f);
        auto const& result = pickSys->GetLastResult();
        if (result.Valid) {
            m_EditModeSystem->OnMouseClick(result.PrimitiveID, relPos.x, relPos.y, isShiftHeld);
        } else {
            // Click on empty space clears selection (unless additive).
            if (!isShiftHeld) {
                m_EditModeSystem->OnMouseClick(0xFFFFFFFFu, relPos.x, relPos.y, false);
            }
        }
        return true;
    }

    // Gizmo gets first right of refusal — prevents mis-selecting while dragging a handle.
    if (m_GizmoRenderer && m_GizmoRenderer->OnMouseButtonPressed(relPos.x, relPos.y)) {
        return true;
    }

    auto* selSystem = m_ActiveScene->GetSystem<SelectionSystem>();
    if (selSystem == nullptr) { return false; }

    LOG_TRACE(
        "EditorLayer click: mouse=({:.0f},{:.0f}) viewportMin=({:.0f},{:.0f}) rel=({:.0f},{:.0f})",
        mousePos.x, mousePos.y, viewportMin.x, viewportMin.y, relPos.x, relPos.y);

    selSystem->OnMouseClick(static_cast<uint32_t>(relPos.x), static_cast<uint32_t>(relPos.y),
                            isShiftHeld);
    return true;
}

bool EditorLayer::OnMouseButtonReleased(MouseButtonReleasedEvent& /*e*/) {
    if (m_GizmoRenderer) { m_GizmoRenderer->OnMouseButtonReleased(); }
    return false;
}

bool EditorLayer::OnMouseMoved(MouseMovedEvent& /*e*/) {
    Vec2 const mousePos = Input::GetMousePosition();
    Vec2 const delta    = mousePos - m_LastMousePosition;
    m_LastMousePosition = mousePos;

    if (!m_ViewportPanel || !m_ViewportPanel->IsHovered()) { return false; }

    if (m_InEditMode && m_EditModeSystem && m_EditModeSystem->IsGrabActive()) {
        m_EditModeSystem->UpdateGrab(delta.x, delta.y);
        return false;
    }

    Vec2 const viewportMin = m_ViewportPanel->GetViewportMin();
    Vec2 const relPos      = mousePos - viewportMin;
    if (relPos.x >= 0.0f && relPos.y >= 0.0f && m_GizmoRenderer) {
        m_GizmoRenderer->OnMouseMoved(relPos.x, relPos.y);
    }
    return false;
}

bool EditorLayer::OnMouseScrolled(MouseScrolledEvent& e) {
    if (!m_ViewportPanel || !m_ViewportPanel->IsHovered()) { return false; }
    m_CameraController->OnScrolled(e.GetYOffset());
    return false;
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent& e) {
    // Block shortcuts only when the user is actively typing in an InputText widget.
    // WantCaptureKeyboard is true for any focused ImGui window (too aggressive);
    // WantTextInput is true only during active text entry.
    if (ImGui::GetIO().WantTextInput) { return false; }

    // RMB held = fly camera mode; suppress all editor shortcuts so WASD/Space/Shift
    // reach the camera controller without also firing gizmo or edit-mode actions.
    if (Input::IsMouseButtonPressed(MouseCode::Right)) { return false; }

    if (e.GetKeyCode() == static_cast<int>(KeyCode::Tab)) {
        if (m_EditModeSystem) {
            auto* selSys = m_ActiveScene->GetSystem<SelectionSystem>();
            auto& selCtx = selSys->GetSelectionContext();
            if (!m_InEditMode) {
                // Prefer the active selection; fall back to the first mesh entity in the scene.
                entt::entity target = selCtx.GetActiveEntity();
                if (target == entt::null || !m_ActiveScene->HasComponent<MeshComponent>(target)) {
                    auto view = m_ActiveScene->GetAllEntitiesWith<MeshComponent>();
                    for (auto e2 : view) {
                        target = e2;
                        break;
                    }
                }
                if (target != entt::null && m_ActiveScene->HasComponent<MeshComponent>(target)) {
                    m_EditModeSystem->Enter(target);
                    selCtx.ClearSelection();
                    m_InEditMode = true;
                    std::string_view name;
                    if (m_ActiveScene->HasComponent<NameComponent>(target)) {
                        name = m_ActiveScene->GetComponent<NameComponent>(target).Name;
                    }
                    if (m_OutlinerPanel) { m_OutlinerPanel->SetInEditMode(true, name); }
                    LOG_INFO("Entered Edit Mode on entity {}", (uint32_t) target);
                }
            } else {
                entt::entity const edited = m_EditModeSystem->GetEditedEntity();
                m_EditModeSystem->Exit();
                m_InEditMode = false;
                if (m_OutlinerPanel) { m_OutlinerPanel->SetInEditMode(false); }
                if (edited != entt::null) { selCtx.Select(edited); }
                LOG_INFO("Exited Edit Mode");
            }
        }
        return true;
    }

    if (e.GetKeyCode() == static_cast<int>(KeyCode::Delete)) {
        if (!m_InEditMode && m_OutlinerPanel) { m_OutlinerPanel->TriggerDeleteConfirmation(); }
        return true;
    }

    // Element mode shortcuts (edit mode only): 1=Vertex, 2=Edge, 3=Face.
    if (m_InEditMode && m_EditModeSystem) {
        if (e.GetKeyCode() == static_cast<int>(KeyCode::D1)) {
            m_EditModeSystem->SetElementMode(ElementMode::Vertex);
            return true;
        }
        if (e.GetKeyCode() == static_cast<int>(KeyCode::D2)) {
            m_EditModeSystem->SetElementMode(ElementMode::Edge);
            return true;
        }
        if (e.GetKeyCode() == static_cast<int>(KeyCode::D3)) {
            m_EditModeSystem->SetElementMode(ElementMode::Face);
            return true;
        }
        if (e.GetKeyCode() == static_cast<int>(KeyCode::E)) {
            if (m_EditModeSystem->IsGrabActive()) {
                m_EditModeSystem->ConfirmGrab();
            } else {
                m_EditModeSystem->Extrude();
            }
            return true;
        }
        if (e.GetKeyCode() == static_cast<int>(KeyCode::Escape)) {
            if (m_EditModeSystem->IsGrabActive()) {
                m_EditModeSystem->CancelGrab();
                return true;
            }
        }
        // Don't return false here — let Ctrl+Z/Y fall through to the undo handler below.
    }

    // Gizmo shortcuts are object-mode only.

    if (e.GetKeyCode() == static_cast<int>(KeyCode::T)) {
        if (m_GizmoRenderer) { m_GizmoRenderer->SetMode(GizmoMode::Translation); }
        return true;
    }
    if (e.GetKeyCode() == static_cast<int>(KeyCode::R)) {
        if (m_GizmoRenderer) { m_GizmoRenderer->SetMode(GizmoMode::Rotation); }
        return true;
    }
    if (e.GetKeyCode() == static_cast<int>(KeyCode::S)) {
        if (m_GizmoRenderer) { m_GizmoRenderer->SetMode(GizmoMode::Scale); }
        return true;
    }
    if (e.GetKeyCode() == static_cast<int>(KeyCode::G)) {
        if (m_GizmoRenderer) { m_GizmoRenderer->ToggleSpace(); }
        return true;
    }

    bool const ctrl =
        Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
    if (!ctrl) { return false; }

    auto* hist = m_ActiveScene->GetSystem<HistorySystem>();
    if (hist == nullptr) { return false; }

    if (e.GetKeyCode() == static_cast<int>(KeyCode::Z)) {
        hist->Undo();
        return true;
    }
    if (e.GetKeyCode() == static_cast<int>(KeyCode::Y)) {
        hist->Redo();
        return true;
    }
    return false;
}

bool EditorLayer::OnKeyReleased(KeyReleasedEvent& /*e*/) {
    return false;
}

bool EditorLayer::OnWindowResize(WindowResizeEvent& /*e*/) {
    // Camera and picking system dimensions are driven exclusively by OnViewportResize, which
    // receives the actual viewport content area from ViewportPanel each frame. Setting them here
    // would use the OS window size instead of the smaller content area, causing a coordinate
    // mismatch between where the user clicks and where the picking FBO stores entity IDs.
    return false;
}
