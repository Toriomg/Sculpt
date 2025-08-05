#pragma once
#include "Platform/Layers/Layer.h"
#include "Platform/System/Input/Input.h"
//#include "Scene/Scene.h"
//#include "UI/EditorUI.h"
#include "Platform/System/Events/KeyboardEvent.h"
#include "Platform/System/Events/MouseEvent.h"

class EditorLayer : public Layer {
public:
    EditorLayer();
    virtual ~EditorLayer() = default;

    virtual void OnAttach() override;
    virtual void OnUpdate() override;
    virtual void OnEvent(Event& event) override;

private:
    bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
    bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);
    bool OnKeyPressed(KeyPressedEvent& e);
    bool OnKeyReleased(KeyReleasedEvent& e);

    // This layer now owns the core application state and UI
    //std::unique_ptr<Scene> m_ActiveScene;
    //std::unique_ptr<EditorUI> m_EditorUI;
};