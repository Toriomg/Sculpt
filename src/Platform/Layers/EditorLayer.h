#pragma once
#include "Platform/Layers/Layer.h"
#include "Platform/System/Input/Input.h"
#include "Core/Scene.h"
//#include "UI/EditorUI.h"
#include "Platform/System/Events/KeyboardEvent.h"
#include "Platform/System/Events/MouseEvent.h"
#include "Platform/System/Events/WindowEvent.h"

#include "Editor/EditorCameraController.h"
// Temp
#include "Renderer/Camera.h"
#include "Renderer/Material.h"
#include "Renderer/Mesh.h"
#include "Renderer/Renderer.h"

class EditorLayer : public Layer {
public:
    EditorLayer();
    virtual ~EditorLayer() = default;

    virtual void OnAttach() override;
    virtual void OnUpdate(float deltaTime) override;
    virtual void OnEvent(Event& event) override;

private:
    bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
    bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);
	bool OnMouseMoved(MouseMovedEvent& e);
    bool OnKeyPressed(KeyPressedEvent& e);
    bool OnKeyReleased(KeyReleasedEvent& e);
    bool OnWindowResize(WindowResizeEvent& e);

    // This layer now owns the core application state and UI
    std::unique_ptr<Scene> m_ActiveScene;
    //std::unique_ptr<EditorUI> m_EditorUI;
    std::unique_ptr<EditorCameraController> m_CameraController;

    Camera m_EditorCamera;
	
    Entity m_CameraEntity;

    Entity m_CubeEntity;
    Entity m_SphereEntity;
    Entity m_PyramidEntity;
    Entity m_TorusEntity;

    // Mouse state for camera rotation
    Vec2 m_LastMousePosition = Vec2( 0.0f, 0.0f );
};