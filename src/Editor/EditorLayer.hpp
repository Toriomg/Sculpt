// Primary editor layer: owns Scene, Camera, EditorCameraController, viewport FBO, and all UI
// panels.
#pragma once
#include "Core/Scene.hpp"
#include "Editor/EditorCameraController.hpp"
#include "Platform/Layers/Layer.hpp"
#include "Platform/System/Events/KeyboardEvent.hpp"
#include "Platform/System/Events/MouseEvent.hpp"
#include "Platform/System/Events/WindowEvent.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/InfGrid.hpp"
#include <functional>
#include <memory>

class Framebuffer;
class EntityFactory;
class GizmoRenderer;
class EditModeSystem;
class ViewportPanel;
class OutlinerPanel;
class InspectorPanel;
class MainMenuBar;
class ScenePanel;

class EditorLayer : public Layer {
public:
    explicit EditorLayer(std::function<void()> onQuit);
    ~EditorLayer() override;
    EditorLayer(EditorLayer const&)            = delete;
    EditorLayer& operator=(EditorLayer const&) = delete;
    EditorLayer(EditorLayer&&)                 = default;
    EditorLayer& operator=(EditorLayer&&)      = default;

    void OnAttach() override;
    void OnUpdate(float deltaTime) override;
    void OnImGuiRender() override;
    void OnEvent(Event& event) override;

private:
    bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
    bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);
    bool OnMouseMoved(MouseMovedEvent& e);
    bool OnMouseScrolled(MouseScrolledEvent& e);
    bool OnKeyPressed(KeyPressedEvent& e);
    static bool OnKeyReleased(KeyReleasedEvent& e);
    static bool OnWindowResize(WindowResizeEvent& e);

    void OnViewportResize(uint32_t width, uint32_t height);

    std::unique_ptr<Scene> m_ActiveScene;
    std::unique_ptr<EditorCameraController> m_CameraController;
    std::unique_ptr<InfGrid> m_Grid;
    std::unique_ptr<Framebuffer> m_ViewportFBO;

    Camera m_EditorCamera;
    Entity m_CameraEntity;

    Vec2 m_LastMousePosition{0.0f, 0.0f};

    bool m_InEditMode = false;
    std::unique_ptr<EditModeSystem> m_EditModeSystem;

    std::unique_ptr<EntityFactory> m_EntityFactory;
    std::unique_ptr<GizmoRenderer> m_GizmoRenderer;
    std::unique_ptr<ViewportPanel> m_ViewportPanel;
    std::unique_ptr<OutlinerPanel> m_OutlinerPanel;
    std::unique_ptr<InspectorPanel> m_InspectorPanel;
    std::unique_ptr<MainMenuBar> m_MainMenuBar;
    std::unique_ptr<ScenePanel> m_ScenePanel;

    std::function<void()> m_OnQuit;
};
