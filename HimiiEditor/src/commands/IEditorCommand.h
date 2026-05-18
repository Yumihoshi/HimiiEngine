#pragma once

namespace Himii
{
    class IEditorCommand
    {
    public:
        virtual ~IEditorCommand() = default;

        virtual void Execute() = 0;
        virtual void Undo() = 0;

        /// 连续同类操作可合并（例如拖拽 Transform）
        virtual bool TryMerge(const IEditorCommand& other)
        {
            (void)other;
            return false;
        }
    };
}
