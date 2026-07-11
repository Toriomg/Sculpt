#include "SelectionSystem.h"
#include "PickingSystem.h"
#include "Core/Scene.h"
#include "Core/Components/Component.h"

void SelectionContext::Select(Entity entity, bool additive) {
    if (!additive) {
        m_SelectedEntities.clear();
    }

    bool wasAdded = m_SelectedEntities.insert(entity).second;
    if (wasAdded && OnSelectionChanged) {
        std::vector<Entity> selected(m_SelectedEntities.begin(), m_SelectedEntities.end());
        OnSelectionChanged(selected);
    }
}

void SelectionContext::Deselect(Entity entity) {
    bool wasRemoved = m_SelectedEntities.erase(entity) > 0;
    if (wasRemoved && OnSelectionChanged) {
        std::vector<Entity> selected(m_SelectedEntities.begin(), m_SelectedEntities.end());
        OnSelectionChanged(selected);
    }
}

void SelectionContext::ClearSelection() {
    if (m_SelectedEntities.empty()) {
        return;
    }

    m_SelectedEntities.clear();
    if (OnSelectionChanged) {
        OnSelectionChanged({});
    }
}

void SelectionContext::SelectMultiple(const std::vector<Entity>& entities, bool additive) {
    if (!additive) {
        m_SelectedEntities.clear();
    }

    bool changed = false;
    for (Entity entity : entities) {
        if (m_SelectedEntities.insert(entity).second) {
            changed = true;
        }
    }

    if (changed && OnSelectionChanged) {
        std::vector<Entity> selected(m_SelectedEntities.begin(), m_SelectedEntities.end());
        OnSelectionChanged(selected);
    }
}

SelectionSystem::SelectionSystem() = default;

void SelectionSystem::OnAttach(Scene* scene) {
    System::OnAttach(scene);
    m_PickingSystem = scene->GetSystem<PickingSystem>();
}

void SelectionSystem::OnUpdate(float deltaTime) {
    if (!m_Scene || !m_PickingSystem) {
        return;
    }
}

void SelectionSystem::OnMouseClick(uint32_t screenX, uint32_t screenY, bool additive) {
    if (!m_PickingSystem) {
        return;
    }

    m_PickingSystem->RequestPickingPass(screenX, screenY);
    m_PickingSystem->OnUpdate(0.0f);

    const auto& result = m_PickingSystem->GetLastResult();

    if (result.Valid) {
        m_SelectionContext.Select(result.SelectedEntity, additive);
    } else {
        if (!additive) {
            m_SelectionContext.ClearSelection();
        }
    }
}

void SelectionSystem::ApplyHighlightShaders() {
}
