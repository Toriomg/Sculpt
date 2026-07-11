#pragma once
#include "System.h"
#include "Core/Entity.h"
#include <unordered_set>
#include <vector>
#include <functional>

class PickingSystem;

class SelectionContext {
public:
    const std::unordered_set<entt::entity>& GetSelectedEntities() const {
        return m_SelectedEntities;
    }

    bool IsEntitySelected(Entity entity) const {
        return m_SelectedEntities.count(entity) > 0;
    }

    size_t GetSelectionCount() const {
        return m_SelectedEntities.size();
    }

    void Select(Entity entity, bool additive = false);
    void Deselect(Entity entity);
    void ClearSelection();
    void SelectMultiple(const std::vector<Entity>& entities, bool additive = false);

    std::function<void(const std::vector<Entity>&)> OnSelectionChanged;

private:
    std::unordered_set<entt::entity> m_SelectedEntities;
};

class SelectionSystem : public System {
public:
    SelectionSystem();
    ~SelectionSystem() override = default;

    void OnAttach(Scene* scene) override;
    void OnUpdate(float deltaTime) override;

    void OnMouseClick(uint32_t screenX, uint32_t screenY, bool additive = false);

    SelectionContext& GetSelectionContext() { return m_SelectionContext; }
    const SelectionContext& GetSelectionContext() const { return m_SelectionContext; }

private:
    void ApplyHighlightShaders();

    SelectionContext m_SelectionContext;
    PickingSystem* m_PickingSystem = nullptr;
};
