#include "SelectionSystem.hpp"
#include "Core/Scene.hpp"
#include "PickingSystem.hpp"

void SelectionContext::NotifyChanged() {
    if (!OnSelectionChanged) { return; }
    std::vector<Entity> selected(m_SelectedEntities.begin(), m_SelectedEntities.end());
    OnSelectionChanged(selected);
}

void SelectionContext::Select(Entity entity, bool additive) {
    if (!additive) { m_SelectedEntities.clear(); }
    bool const wasAdded = m_SelectedEntities.insert(entity).second;
    m_ActiveEntity      = entity;
    if (wasAdded) { NotifyChanged(); }
}

void SelectionContext::Deselect(Entity entity) {
    bool const wasRemoved = m_SelectedEntities.erase(entity) > 0;
    if (!wasRemoved) { return; }
    if (m_ActiveEntity == entity) {
        m_ActiveEntity = m_SelectedEntities.empty() ? entt::null : *m_SelectedEntities.begin();
    }
    NotifyChanged();
}

void SelectionContext::ClearSelection() {
    if (m_SelectedEntities.empty()) { return; }
    m_SelectedEntities.clear();
    m_ActiveEntity = entt::null;
    NotifyChanged();
}

void SelectionContext::SelectMultiple(std::vector<Entity> const& entities, bool additive) {
    if (!additive) { m_SelectedEntities.clear(); }
    bool changed = false;
    for (Entity const entity : entities) {
        if (m_SelectedEntities.insert(entity).second) { changed = true; }
    }
    if (!entities.empty()) { m_ActiveEntity = entities.back(); }
    if (changed) { NotifyChanged(); }
}

SelectionSystem::SelectionSystem() = default;

void SelectionSystem::OnAttach(Scene* scene) {
    System::OnAttach(scene);
    m_PickingSystem = scene->GetSystem<PickingSystem>();
}

void SelectionSystem::OnUpdate(float /*deltaTime*/) { }

void SelectionSystem::OnMouseClick(uint32_t screenX, uint32_t screenY, bool additive) {
    if (m_PickingSystem == nullptr) { return; }

    LOG_TRACE("SelectionSystem::OnMouseClick screen=({},{})", screenX, screenY);

    // Force the picking pass to execute immediately (synchronously) instead of waiting for the
    // next Scene::OnUpdate, so the result is available in this same call stack.
    m_PickingSystem->RequestPickingPass(screenX, screenY);
    m_PickingSystem->OnUpdate(0.0f);

    auto const& result = m_PickingSystem->GetLastResult();

    LOG_TRACE("SelectionSystem: result valid={} objectID={}", result.Valid, result.ObjectID);

    if (result.Valid) {
        auto const entity = static_cast<Entity>(result.ObjectID - 1u);
        m_SelectionContext.Select(entity, additive);
    } else {
        if (!additive) { m_SelectionContext.ClearSelection(); }
    }
}

void SelectionSystem::ApplyHighlightShaders() { }
