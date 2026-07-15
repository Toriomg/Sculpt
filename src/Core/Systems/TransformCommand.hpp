// Records a single entity transform change; Execute applies the new state, Undo restores the old.
#pragma once
#include "Core/Systems/Command.hpp"
#include "Core/Entity.hpp"
#include "Platform/CoreUtils/Math/matx.hpp"

class Scene;

class TransformCommand : public Command {
public:
    TransformCommand(Scene* scene, Entity entity, Matx4f before, Matx4f after);

    void        Execute()              override;
    void        Undo()                 override;
    std::string GetDescription() const override { return "Move"; }

private:
    Scene*  m_Scene;
    Entity  m_Entity;
    Matx4f  m_Before;
    Matx4f  m_After;
};
