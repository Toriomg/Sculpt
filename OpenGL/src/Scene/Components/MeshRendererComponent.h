
#pragma once

#include <memory>
#include <vector>

#include "Component.h"

class Mesh;
class Material;

class MeshRendererComponent : public Component {
public:
    // The constructor takes shared pointers to the rendering resources.
    MeshRendererComponent(
        const std::shared_ptr<Mesh>& mesh,
        const std::shared_ptr<Material>& material
    );

    // Add textures to the component after creation.
    void AddTexture(std::shared_ptr<Texture> texture);

    // Public members for the main Renderer to access.
    std::shared_ptr<Mesh> m_Mesh;
    std::shared_ptr<Material> m_Material;

    // This component itself doesn't do anything on update.
    // A script component attached to the same GameObject might, for example.
    void OnUpdate(float deltaTime) override {}

    // The OnRender method is also empty. Why? Because a central 'Renderer' class
    // should be responsible for drawing, not the component itself. The Renderer
    // will ask this component for its data (VAO, Shader, etc.).
    void OnRender() override {}
};