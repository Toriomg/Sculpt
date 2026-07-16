#include "Core/Systems/HistorySystem.hpp"
#include "Core/Systems/Commands/Command.hpp"

void HistorySystem::Push(std::unique_ptr<Command> cmd) {
    cmd->Execute();
    if (m_UndoStack.size() >= k_MaxHistory) m_UndoStack.erase(m_UndoStack.begin());
    m_UndoStack.push_back(std::move(cmd));
    m_RedoStack.clear();
}

void HistorySystem::Undo() {
    if (m_UndoStack.empty()) return;
    m_UndoStack.back()->Undo();
    m_RedoStack.push_back(std::move(m_UndoStack.back()));
    m_UndoStack.pop_back();
}

void HistorySystem::Redo() {
    if (m_RedoStack.empty()) return;
    m_RedoStack.back()->Execute();
    m_UndoStack.push_back(std::move(m_RedoStack.back()));
    m_RedoStack.pop_back();
}

std::string HistorySystem::GetUndoDescription() const {
    return m_UndoStack.empty() ? std::string{} : m_UndoStack.back()->GetDescription();
}

std::string HistorySystem::GetRedoDescription() const {
    return m_RedoStack.empty() ? std::string{} : m_RedoStack.back()->GetDescription();
}

void HistorySystem::Clear() {
    m_UndoStack.clear();
    m_RedoStack.clear();
}
