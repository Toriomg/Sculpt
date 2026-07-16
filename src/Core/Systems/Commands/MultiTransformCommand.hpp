// Records N entity transforms as one undoable step.
#pragma once
#include "Core/Components/Component.hpp"
#include "Core/Entity.hpp"
#include "Core/Systems/Commands/Command.hpp"
#include <utility>
#include <vector>

class Scene;

class MultiTransformCommand : public Command {
public:
    MultiTransformCommand(Scene* scene, std::vector<std::pair<Entity, TransformComponent>> before,
                          std::vector<std::pair<Entity, TransformComponent>> after);

    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string GetDescription() const override { return "Transform"; }

private:
    Scene* m_Scene;
    std::vector<std::pair<Entity, TransformComponent>> m_Before;
    std::vector<std::pair<Entity, TransformComponent>> m_After;
};
