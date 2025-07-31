#pragma once
#include "Platform/Layers/Layer.h"
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
    // Event handlers now live here!
    bool OnKeyPressed(KeyPressedEvent& e);
    bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

    // This layer now owns the core application state and UI
    //std::unique_ptr<Scene> m_ActiveScene;
    //std::unique_ptr<EditorUI> m_EditorUI;
};