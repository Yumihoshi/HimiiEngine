#pragma once

#include "IEditorCommand.h"
#include <memory>
#include <vector>

namespace Himii
{
    class EditorCommandHistory
    {
    public:
        explicit EditorCommandHistory(size_t maximumDepth = 50);

        void Execute(std::unique_ptr<IEditorCommand> command);
        bool Undo();
        bool Redo();
        void Clear();

        bool CanUndo() const { return !m_UndoStack.empty(); }
        bool CanRedo() const { return !m_RedoStack.empty(); }

    private:
        size_t m_MaximumDepth;
        std::vector<std::unique_ptr<IEditorCommand>> m_UndoStack;
        std::vector<std::unique_ptr<IEditorCommand>> m_RedoStack;
    };
}
