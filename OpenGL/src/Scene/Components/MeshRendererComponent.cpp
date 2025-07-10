#include "MeshRendererComponent.h"

MeshRendererComponent::MeshRendererComponent(
    std::shared_ptr<VertexArray> vao,
    std::shared_ptr<IndexBuffer> ibo,
    std::shared_ptr<Shader> shader)
    : m_VAO(vao), m_IBO(ibo), m_Shader(shader)
{
    // The member variables are initialized in the initializer list.
}

void MeshRendererComponent::AddTexture(std::shared_ptr<Texture> texture)
{
    Textures.push_back(texture);
}