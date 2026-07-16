#include "Core/Systems/Commands/TransformCommand.hpp"
#include "Core/Components/Component.hpp"
#include "Core/Scene.hpp"

TransformCommand::TransformCommand(Scene* scene, Entity entity, TransformComponent before,
                                   TransformComponent after)
    : m_Scene(scene), m_Entity(entity), m_Before(before), m_After(after) { }

void TransformCommand::Execute() {
    if (m_Scene->HasComponent<TransformComponent>(m_Entity))
        m_Scene->GetComponent<TransformComponent>(m_Entity) = m_After;
}

void TransformCommand::Undo() {
    if (m_Scene->HasComponent<TransformComponent>(m_Entity))
        m_Scene->GetComponent<TransformComponent>(m_Entity) = m_Before;
}
