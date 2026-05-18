#pragma once

#include "Himii/Scripting/ScriptIDE.h"

namespace Himii
{
    class ProjectSettingsPanel
    {
    public:
        void OnImGuiRender(bool* pOpen);
        void Reset() { m_Initialized = false; }

    private:
        ScriptIDEType m_TempIDEOverride = ScriptIDEType::Inherit;
        char m_TempCustomPath[512] = "";
        char m_TempCustomArgs[512] = "";
        bool m_Initialized = false;

        void SyncFromProject();
        void ApplyToProject();
    };
}
