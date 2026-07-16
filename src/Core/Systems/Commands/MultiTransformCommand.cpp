#include "Core/Systems/Commands/MultiTransformCommand.hpp"
#include "Core/Scene.hpp"

MultiTransformCommand::MultiTransformCommand(
    Scene* scene, std::vector<std::pair<Entity, TransformComponent>> before,
    std::vector<std::pair<Entity, TransformComponent>> after)
    : m_Scene(scene), m_Before(std::move(before)), m_After(std::move(after)) { }

MultiTransformCommand::MultiTransformCommand(Scene* scene, Entity entity, TransformComponent before,
                                             TransformComponent after)
    : m_Scene(scene), m_Before({
                        {entity, before}
}),
      m_After({{entity, after}}) { }

void MultiTransformCommand::Execute() {
    for (auto const& [entity, tc] : m_After) {
        if (m_Scene->HasComponent<TransformComponent>(entity)) {
            m_Scene->GetComponent<TransformComponent>(entity) = tc;
        }
    }
}

void MultiTransformCommand::Undo() {
    for (auto const& [entity, tc] : m_Before) {
        if (m_Scene->HasComponent<TransformComponent>(entity)) {
            m_Scene->GetComponent<TransformComponent>(entity) = tc;
        }
    }
}
