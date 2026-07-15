#include "Core/Systems/TransformCommand.hpp"
#include "Core/Scene.hpp"
#include "Core/Components/Component.hpp"

TransformCommand::TransformCommand(Scene* scene, Entity entity, Matx4f before, Matx4f after)
    : m_Scene(scene), m_Entity(entity), m_Before(before), m_After(after) {}

void TransformCommand::Execute() {
    if (m_Scene->HasComponent<TransformComponent>(m_Entity))
        m_Scene->GetComponent<TransformComponent>(m_Entity).Transform = m_After;
}

void TransformCommand::Undo() {
    if (m_Scene->HasComponent<TransformComponent>(m_Entity))
        m_Scene->GetComponent<TransformComponent>(m_Entity).Transform = m_Before;
}
