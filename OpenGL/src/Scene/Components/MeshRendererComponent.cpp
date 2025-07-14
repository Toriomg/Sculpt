#include "MeshRendererComponent.h"

MeshRendererComponent::MeshRendererComponent(
    const std::shared_ptr<Mesh>& mesh,
    const std::shared_ptr<Material>& material)
    : m_Mesh(mesh), m_Material(material)
{
    // The member variables are initialized in the initializer list.
}