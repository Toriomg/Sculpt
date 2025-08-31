#pragma once
#include "System.h"

class Renderer;

class RenderingSystem : public System
{
public:
    RenderingSystem() = default;
    ~RenderingSystem() = default;

    // This is the main logic function that will be called by the Scene every frame.
    virtual void OnUpdate(float deltaTime) override;
};