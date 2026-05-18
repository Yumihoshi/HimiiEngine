#pragma once

#include "Himii/Scripting/ScriptIDE.h"

namespace Himii
{
    class EditorPreferencesPanel
    {
    public:
        void OnImGuiRender(bool* pOpen);

    private:
        void DrawIDEConfig(ScriptIDEConfig& config, const char* idSuffix);
    };
}
