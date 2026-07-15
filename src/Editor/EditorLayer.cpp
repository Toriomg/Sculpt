#include "Editor/EditorLayer.hpp"
#include "Editor/Gizmos/GizmoRenderer.hpp"
#include "Editor/Panels/ViewportPanel.hpp"
#include "Editor/Panels/OutlinerPanel.hpp"
#include "Editor/Panels/InspectorPanel.hpp"
#include "Editor/Panels/MainMenuBar.hpp"
#include "Editor/Panels/ScenePanel.hpp"
#include "Core/Components/Component.hpp"
#include "Core/Systems/SelectionSystem.hpp"
#include "Core/Systems/PickingSystem.hpp"
#include "Core/Systems/HistorySystem.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Renderer.hpp"
#include "Platform/Graphics/Framebuffer.hpp"
#include "Platform/Graphics/RenderCommand.hpp"
#include "Platform/Graphics/Shader.hpp"
#include "Platform/System/Input/Input.hpp"
#include "Platform/System/Input/KeyCodes.hpp"

EditorLayer::EditorLayer(std::function<void()> onQuit)
    : Layer("EditorLayer"), m_OnQuit(std::move(onQuit)) {}

// Destructor defined here so forward-declared types (Framebuffer, panels) are fully visible.
EditorLayer::~EditorLayer() = default;

void EditorLayer::OnAttach() {
    CORE_LOG_INFO("EditorLayer attached!");

    m_ActiveScene = std::make_unique<Scene>();

    m_CameraEntity = m_ActiveScene->CreateGameObject("Main Camera");
    auto& camComp = m_ActiveScene->AddComponent<CameraComponent>(m_CameraEntity);
    auto& camTransform = m_ActiveScene->GetComponent<TransformComponent>(m_CameraEntity);
    camTransform.Transform = Matx4f::translation(Vec3(0.0f, 0.0f, 5.0f));
    camComp.SceneCamera.SetPosition({ 0.0f, 0.0f, 5.0f });

    auto SphereMesh   = Mesh::CreateSphere(1.0f, 32, 32);
    auto PyramidMesh  = Mesh::CreateIcosahedron(2.0f);
    auto TorusMesh    = Mesh::CreateTorus(1.0f, 0.5f, 32, 32);

    auto simpleShader = std::make_shared<Shader>("res/shaders/modelmesh.shader");
    auto myMaterial   = std::make_shared<Material>(simpleShader);

    Vec3 MonkeyPosition  = {  0.0f, 0.0f, 0.0f };
    Vec3 SpherePosition  = {  5.0f, 0.0f, 0.0f };
    Vec3 PyramidPosition = { -5.0f, 0.0f, 0.0f };
    Vec3 TorusPosition   = { 10.0f, 0.0f, 0.0f };

    m_MonkeyEntity  = m_ActiveScene->CreateGameObject("Monkey");
    m_SphereEntity  = m_ActiveScene->CreateGameObject("Sphere");
    m_PyramidEntity = m_ActiveScene->CreateGameObject("Pyramid");
    m_TorusEntity   = m_ActiveScene->CreateGameObject("Torus");

    // Procedural meshes need no file I/O — add components immediately.
    m_ActiveScene->AddComponent<MeshComponent>(m_SphereEntity, SphereMesh, myMaterial);
    m_ActiveScene->SetComponent<TransformComponent>(m_SphereEntity, Matx4f::translation(SpherePosition));
    m_ActiveScene->AddComponent<SelectionComponent>(m_SphereEntity);

    m_ActiveScene->AddComponent<MeshComponent>(m_PyramidEntity, PyramidMesh, myMaterial);
    m_ActiveScene->SetComponent<TransformComponent>(m_PyramidEntity, Matx4f::translation(PyramidPosition));
    m_ActiveScene->AddComponent<SelectionComponent>(m_PyramidEntity);

    m_ActiveScene->AddComponent<MeshComponent>(m_TorusEntity, TorusMesh, myMaterial);
    m_ActiveScene->SetComponent<TransformComponent>(m_TorusEntity, Matx4f::translation(TorusPosition));
    m_ActiveScene->AddComponent<SelectionComponent>(m_TorusEntity);

    // Transform and selection are set immediately; MeshComponent is added once Assimp finishes.
    m_ActiveScene->SetComponent<TransformComponent>(m_MonkeyEntity, Matx4f::translation(MonkeyPosition));
    m_ActiveScene->AddComponent<SelectionComponent>(m_MonkeyEntity);
    AssetManager::LoadAsync("res/models/dragon.obj", [this, myMaterial](AssetHandle handle) {
        if (auto mesh = AssetManager::GetAs<Mesh>(handle))
            m_ActiveScene->AddComponent<MeshComponent>(m_MonkeyEntity, mesh, myMaterial);
    });

    // Texture handle is set on the shared material once stb_image finishes; all entities using
    // this material pick it up automatically at the next Renderer::Submit call.
    AssetManager::LoadAsync("res/textures/texture1.png", [myMaterial](AssetHandle handle) {
        myMaterial->SetTexture(handle);
    });

    // Stores a raw pointer into the ECS component — safe while the entity lives.
    m_CameraController = std::make_unique<EditorCameraController>(&camComp.SceneCamera);
    m_Grid = std::make_unique<InfGrid>();
    m_ViewportFBO = std::make_unique<Framebuffer>(1470, 810);

    auto* selSys = m_ActiveScene->GetSystem<SelectionSystem>();

    m_GizmoRenderer = std::make_unique<GizmoRenderer>(
        *m_ActiveScene,
        selSys->GetSelectionContext(),
        camComp.SceneCamera
    );
    m_GizmoRenderer->OnAttach();

    m_ViewportPanel  = std::make_unique<ViewportPanel>(
        m_ViewportFBO.get(),
        [this](uint32_t w, uint32_t h) { OnViewportResize(w, h); }
    );
    m_OutlinerPanel  = std::make_unique<OutlinerPanel>(m_ActiveScene.get(), selSys);
    m_InspectorPanel = std::make_unique<InspectorPanel>(
        m_ActiveScene.get(),
        &selSys->GetSelectionContext()
    );
    m_MainMenuBar = std::make_unique<MainMenuBar>(
        m_OnQuit,
        m_ActiveScene->GetSystem<HistorySystem>(),
        m_OutlinerPanel.get(),
        m_InspectorPanel.get(),
        m_ScenePanel.get(),
        m_ViewportPanel.get()
    );
    m_ScenePanel  = std::make_unique<ScenePanel>(&camComp.SceneCamera);
}

void EditorLayer::OnViewportResize(uint32_t width, uint32_t height) {
    m_ViewportFBO->Resize(width, height);
    auto& camComp = m_ActiveScene->GetComponent<CameraComponent>(m_CameraEntity);
    camComp.SceneCamera.SetViewportSize(width, height);
    RenderCommand::SetViewport(0, 0, width, height);
    auto* pickSys = m_ActiveScene->GetSystem<PickingSystem>();
    if (pickSys)
        pickSys->OnWindowResize(width, height);
    if (m_GizmoRenderer)
        m_GizmoRenderer->SetViewportSize(width, height);
}

void EditorLayer::OnUpdate(float deltaTime) {
    auto& camComp     = m_ActiveScene->GetComponent<CameraComponent>(m_CameraEntity);
    auto& camTransform = m_ActiveScene->GetComponent<TransformComponent>(m_CameraEntity);

    // Camera movement is gated on viewport focus so ImGui text fields can receive keyboard input.
    if (!m_ViewportPanel || m_ViewportPanel->IsFocused())
        m_CameraController->OnUpdate(deltaTime);

    // Sync TransformComponent to camera position driven by the controller.
    camTransform.Transform = Matx4f::translation(camComp.SceneCamera.GetPosition());

    // Apply the scene panel's global transform on top of each entity's local transform.
    // m_PrevGlobalInv undoes last frame's global, recovering any Inspector edits made
    // between frames. The new global is then applied and its inverse saved for next frame.
    if (m_ScenePanel) {
        Matx4f curGlobal = m_ScenePanel->GetGlobalTransform();
        auto view = m_ActiveScene->GetAllEntitiesWith<MeshComponent, TransformComponent>();
        for (auto entity : view) {
            auto& tc = m_ActiveScene->GetComponent<TransformComponent>(entity);
            tc.Transform = curGlobal * (m_PrevGlobalInv * tc.Transform);
        }
        m_PrevGlobalInv = m_ScenePanel->GetInverseGlobalTransform();
    }

    // Render the scene into the viewport FBO.
    m_ViewportFBO->Bind();
    m_ActiveScene->OnUpdate(deltaTime);
    m_Grid->Draw(
        camComp.SceneCamera.GetViewMatrix(),
        camComp.SceneCamera.GetProjectionMatrix(),
        camComp.SceneCamera.GetPosition()
    );
    m_GizmoRenderer->Draw();
    m_ViewportFBO->Unbind();
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
    dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& e)  { return OnMouseButtonPressed(e); });
    dispatcher.Dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent& e) { return OnMouseButtonReleased(e); });
    dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e)                  { return OnKeyPressed(e); });
    dispatcher.Dispatch<KeyReleasedEvent>([this](KeyReleasedEvent& e)                { return OnKeyReleased(e); });
    dispatcher.Dispatch<MouseMovedEvent>([this](MouseMovedEvent& e)                  { return OnMouseMoved(e); });
    dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e)            { return OnMouseScrolled(e); });
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e)              { return OnWindowResize(e); });
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e) {
    if (e.GetMouseButton() != 0)
        return false;
    if (!m_ViewportPanel || !m_ViewportPanel->IsHovered())
        return false;

    Vec2 mousePos    = Input::GetMousePosition();
    Vec2 viewportMin = m_ViewportPanel->GetViewportMin();
    Vec2 relPos      = mousePos - viewportMin;

    if (relPos.x < 0.0f || relPos.y < 0.0f)
        return false;

    // Gizmo gets first right of refusal — prevents mis-selecting while dragging a handle.
    if (m_GizmoRenderer && m_GizmoRenderer->OnMouseButtonPressed(relPos.x, relPos.y))
        return true;

    bool isShiftHeld = Input::IsKeyPressed(KeyCode::LeftShift);
    auto* selSystem  = m_ActiveScene->GetSystem<SelectionSystem>();
    if (!selSystem)
        return false;

    LOG_TRACE("EditorLayer click: mouse=({:.0f},{:.0f}) viewportMin=({:.0f},{:.0f}) rel=({:.0f},{:.0f})",
        mousePos.x, mousePos.y, viewportMin.x, viewportMin.y, relPos.x, relPos.y);

    selSystem->OnMouseClick(
        static_cast<uint32_t>(relPos.x),
        static_cast<uint32_t>(relPos.y),
        isShiftHeld
    );
    return true;
}

bool EditorLayer::OnMouseButtonReleased(MouseButtonReleasedEvent& e) {
    if (m_GizmoRenderer) m_GizmoRenderer->OnMouseButtonReleased();
    return false;
}

bool EditorLayer::OnMouseMoved(MouseMovedEvent& e) {
    if (!m_ViewportPanel || !m_ViewportPanel->IsHovered()) return false;
    Vec2 mousePos    = Input::GetMousePosition();
    Vec2 viewportMin = m_ViewportPanel->GetViewportMin();
    Vec2 relPos      = mousePos - viewportMin;
    if (relPos.x >= 0.0f && relPos.y >= 0.0f && m_GizmoRenderer)
        m_GizmoRenderer->OnMouseMoved(relPos.x, relPos.y);
    return false;
}

bool EditorLayer::OnMouseScrolled(MouseScrolledEvent& e) {
    if (!m_ViewportPanel || !m_ViewportPanel->IsHovered())
        return false;
    m_CameraController->OnScrolled(e.GetYOffset());
    return false;
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent& e) {
    bool ctrl = Input::IsKeyPressed(KeyCode::LeftControl)
             || Input::IsKeyPressed(KeyCode::RightControl);
    if (!ctrl) return false;

    auto* hist = m_ActiveScene->GetSystem<HistorySystem>();
    if (!hist) return false;

    if (e.GetKeyCode() == static_cast<int>(KeyCode::Z)) { hist->Undo(); return true; }
    if (e.GetKeyCode() == static_cast<int>(KeyCode::Y)) { hist->Redo(); return true; }
    return false;
}

bool EditorLayer::OnKeyReleased(KeyReleasedEvent& e) {
    return false;
}

bool EditorLayer::OnWindowResize(WindowResizeEvent& e) {
    // Camera and picking system dimensions are driven exclusively by OnViewportResize, which
    // receives the actual viewport content area from ViewportPanel each frame. Setting them here
    // would use the OS window size instead of the smaller content area, causing a coordinate
    // mismatch between where the user clicks and where the picking FBO stores entity IDs.
    return false;
}
