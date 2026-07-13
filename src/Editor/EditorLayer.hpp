// Primary editor layer: owns Scene, Camera, EditorCameraController, viewport FBO, and all UI panels.
#pragma once
#include "Platform/Layers/Layer.hpp"
#include "Core/Scene.hpp"
#include "Platform/System/Events/KeyboardEvent.hpp"
#include "Platform/System/Events/MouseEvent.hpp"
#include "Platform/System/Events/WindowEvent.hpp"
#include "Editor/EditorCameraController.hpp"
#include "Renderer/InfGrid.hpp"
#include "Renderer/Camera.hpp"
#include <functional>
#include <memory>
#include <unordered_map>

class Framebuffer;
class ViewportPanel;
class OutlinerPanel;
class InspectorPanel;
class MainMenuBar;
class ScenePanel;

class EditorLayer : public Layer {
public:
    explicit EditorLayer(std::function<void()> onQuit);
    ~EditorLayer() override;
    EditorLayer(const EditorLayer&)            = delete;
    EditorLayer& operator=(const EditorLayer&) = delete;
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
    bool OnKeyReleased(KeyReleasedEvent& e);
    bool OnWindowResize(WindowResizeEvent& e);

    void OnViewportResize(uint32_t width, uint32_t height);

    std::unique_ptr<Scene>                  m_ActiveScene;
    std::unique_ptr<EditorCameraController> m_CameraController;
    std::unique_ptr<InfGrid>               m_Grid;
    std::unique_ptr<Framebuffer>           m_ViewportFBO;

    Camera m_EditorCamera;
    Entity m_CameraEntity;
    Entity m_MonkeyEntity;
    Entity m_SphereEntity;
    Entity m_PyramidEntity;
    Entity m_TorusEntity;

    Vec2 m_LastMousePosition{0.0f, 0.0f};

    std::unordered_map<uint32_t, Matx4f> m_BaseTransforms;

    std::unique_ptr<ViewportPanel>  m_ViewportPanel;
    std::unique_ptr<OutlinerPanel>  m_OutlinerPanel;
    std::unique_ptr<InspectorPanel> m_InspectorPanel;
    std::unique_ptr<MainMenuBar>    m_MainMenuBar;
    std::unique_ptr<ScenePanel>     m_ScenePanel;

    std::function<void()> m_OnQuit;
};
