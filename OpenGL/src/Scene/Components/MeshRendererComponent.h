
#pragma once

#include <memory>
#include <vector>

#include "Component.h"
#include "../../Graphics/VertexArray.h"
#include "../../Graphics/IndexBuffer.h"
#include "../../Graphics/Shader.h"
#include "../../Graphics/Texture.h"

class MeshRendererComponent : public Component {
public:
    // The constructor takes shared pointers to the rendering resources.
    MeshRendererComponent(
        std::shared_ptr<VertexArray> vao,
        std::shared_ptr<IndexBuffer> ibo,
        std::shared_ptr<Shader> shader
    );

    // Add textures to the component after creation.
    void AddTexture(std::shared_ptr<Texture> texture);

    // These will be public so the master Renderer can access them easily.
    std::shared_ptr<VertexArray> m_VAO;
    std::shared_ptr<IndexBuffer> m_IBO;
    std::shared_ptr<Shader> m_Shader;
    std::vector<std::shared_ptr<Texture>> Textures;

    // This component itself doesn't do anything on update.
    // A script component attached to the same GameObject might, for example.
    void OnUpdate(float deltaTime) override {}

    // The OnRender method is also empty. Why? Because a central 'Renderer' class
    // should be responsible for drawing, not the component itself. The Renderer
    // will ask this component for its data (VAO, Shader, etc.).
    void OnRender() override {}
};