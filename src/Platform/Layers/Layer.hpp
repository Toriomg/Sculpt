// Abstract base for all application layers; LayerStack drives OnAttach, OnDetach, OnUpdate, and OnEvent each frame.
#pragma once

#include "Platform/System/Events/Event.hpp"
#include "Platform/CoreUtils/Log.hpp"
#include "Core/glhead.hpp"

class Layer
{
public:
    Layer(const std::string& name = "Layer") : m_DebugName(name) {}
    virtual ~Layer() = default;

    // Called when the layer is added to the stack
    virtual void OnAttach() {}
    // Called when the layer is removed from the stack
    virtual void OnDetach() {}
    // Called every frame to update the layer's logic
    virtual void OnUpdate(float deltaTime) {}
    // Called when an event is sent to the layer
    virtual void OnEvent(Event& event) {}

protected:
    std::string m_DebugName;
};