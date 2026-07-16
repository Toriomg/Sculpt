// Manages selection state via SelectionContext; delegates pixel hit-testing to PickingSystem on
// mouse click.
#pragma once
#include "Core/Entity.hpp"
#include "System.hpp"
#include <functional>
#include <unordered_set>
#include <vector>

class PickingSystem;

class SelectionContext {
public:
    std::unordered_set<entt::entity> const& GetSelectedEntities() const {
        return m_SelectedEntities;
    }

    bool IsEntitySelected(Entity entity) const { return m_SelectedEntities.contains(entity); }

    size_t GetSelectionCount() const { return m_SelectedEntities.size(); }

    void Select(Entity entity, bool additive = false);
    void Deselect(Entity entity);
    void ClearSelection();
    void SelectMultiple(std::vector<Entity> const& entities, bool additive = false);

    Entity GetActiveEntity() const { return m_ActiveEntity; }

    std::function<void(std::vector<Entity> const&)> OnSelectionChanged;

private:
    std::unordered_set<entt::entity> m_SelectedEntities;
    entt::entity m_ActiveEntity = entt::null;
};

class SelectionSystem : public System {
public:
    SelectionSystem();
    ~SelectionSystem() override                        = default;
    SelectionSystem(SelectionSystem const&)            = delete;
    SelectionSystem& operator=(SelectionSystem const&) = delete;
    SelectionSystem(SelectionSystem&&)                 = delete;
    SelectionSystem& operator=(SelectionSystem&&)      = delete;

    void OnAttach(Scene* scene) override;
    void OnUpdate(float deltaTime) override;

    void OnMouseClick(uint32_t screenX, uint32_t screenY, bool additive = false);

    SelectionContext& GetSelectionContext() { return m_SelectionContext; }
    SelectionContext const& GetSelectionContext() const { return m_SelectionContext; }

private:
    void ApplyHighlightShaders();

    SelectionContext m_SelectionContext;
    PickingSystem* m_PickingSystem = nullptr;
};
