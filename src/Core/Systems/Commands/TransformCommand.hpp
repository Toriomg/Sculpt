// Records a single entity transform change; Execute applies the new state, Undo restores the old.
#pragma once
#include "Core/Components/Component.hpp"
#include "Core/Entity.hpp"
#include "Core/Systems/Commands/Command.hpp"

class Scene;

class TransformCommand : public Command {
public:
    TransformCommand(Scene* scene, Entity entity, TransformComponent before,
                     TransformComponent after);

    void Execute() override;
    void Undo() override;
    std::string GetDescription() const override { return "Move"; }

private:
    Scene* m_Scene;
    Entity m_Entity;
    TransformComponent m_Before;
    TransformComponent m_After;
};
