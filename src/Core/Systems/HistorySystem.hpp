// Undo/redo stack; Push executes a command and records it, Undo/Redo walk the stacks.
#pragma once
#include "Core/Systems/Commands/Command.hpp"
#include "Core/Systems/System.hpp"
#include <memory>
#include <string>
#include <vector>

class HistorySystem : public System {
public:
    void OnUpdate(float) override { }

    // Execute cmd immediately and push onto the undo stack; clears the redo stack.
    void Push(std::unique_ptr<Command> cmd);

    void Undo();
    void Redo();

    bool CanUndo() const { return !m_UndoStack.empty(); }
    bool CanRedo() const { return !m_RedoStack.empty(); }
    std::string GetUndoDescription() const;
    std::string GetRedoDescription() const;

    void Clear();

private:
    static constexpr std::size_t k_MaxHistory = 100;

    std::vector<std::unique_ptr<Command>> m_UndoStack;
    std::vector<std::unique_ptr<Command>> m_RedoStack;
};
