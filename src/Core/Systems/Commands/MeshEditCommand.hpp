// Undoable mesh topology edit (Extrude, Inset, Loop Cut, etc.); stores full vertex/index snapshots.
#pragma once
#include "Core/EditMesh/EditMesh.hpp"
#include "Core/Entity.hpp"
#include "Core/Systems/Commands/Command.hpp"
#include <string>
#include <vector>

class Scene;
class EditModeSystem;

class MeshEditCommand : public Command {
public:
    MeshEditCommand(Scene* scene, entt::entity entity, EditModeSystem* editSystem,
                    std::vector<EditVertex> vertsBefore, std::vector<uint32_t> indsBefore,
                    std::vector<EditVertex> vertsAfter, std::vector<uint32_t> indsAfter,
                    std::string description = "Extrude");

    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string GetDescription() const override { return m_Description; }

private:
    void Apply(std::vector<EditVertex> const& verts, std::vector<uint32_t> const& inds) const;

    Scene* m_Scene;
    entt::entity m_Entity;
    EditModeSystem* m_EditSystem;
    std::vector<EditVertex> m_Before;
    std::vector<EditVertex> m_After;
    std::vector<uint32_t> m_IndsBefore;
    std::vector<uint32_t> m_IndsAfter;
    std::string m_Description;
};
