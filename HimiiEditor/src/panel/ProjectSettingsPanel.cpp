#include "Hepch.h"
#include "ProjectSettingsPanel.h"
#include "Himii/Project/Project.h"
#include "Himii/Utils/PlatformUtils.h"
#include "Himii/Scripting/ScriptIDE.h"

#include <imgui.h>
#include <cstring>

namespace Himii
{
    static const char* kProjectIDELabels[] = { "Use Editor Default", "Visual Studio", "VS Code", "Rider", "Custom" };
    static const ScriptIDEType kProjectIDEValues[] = {
        ScriptIDEType::Inherit, ScriptIDEType::VisualStudio, ScriptIDEType::VSCode,
        ScriptIDEType::Rider, ScriptIDEType::Custom
    };

    void ProjectSettingsPanel::SyncFromProject()
    {
        if (!Project::GetActive())
            return;

        const auto& config = Project::GetConfig();
        m_TempIDEOverride = config.ScriptIDEOverride;
        strncpy(m_TempCustomPath, config.CustomScriptIDEPath.c_str(), sizeof(m_TempCustomPath) - 1);
        strncpy(m_TempCustomArgs, config.CustomScriptIDEArguments.c_str(), sizeof(m_TempCustomArgs) - 1);
    }

    void ProjectSettingsPanel::ApplyToProject()
    {
        if (!Project::GetActive())
            return;

        auto& config = Project::GetConfig();
        config.ScriptIDEOverride = m_TempIDEOverride;
        config.CustomScriptIDEPath = m_TempCustomPath;
        config.CustomScriptIDEArguments = m_TempCustomArgs;
    }

    void ProjectSettingsPanel::OnImGuiRender(bool* pOpen)
    {
        if (!Project::GetActive())
            return;

        if (ImGui::IsWindowAppearing() || !m_Initialized)
        {
            SyncFromProject();
            m_Initialized = true;
        }

        ImGui::SetNextWindowSize(ImVec2(480, 280), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Project Settings", pOpen))
        {
            ImGui::End();
            return;
        }

        ImGui::Text("Script IDE (Project Override)");
        ImGui::Separator();

        int currentType = 0;
        for (int i = 0; i < 5; ++i)
        {
            if (kProjectIDEValues[i] == m_TempIDEOverride)
            {
                currentType = i;
                break;
            }
        }

        if (ImGui::Combo("Script IDE", &currentType, kProjectIDELabels, 5))
            m_TempIDEOverride = kProjectIDEValues[currentType];

        if (m_TempIDEOverride == ScriptIDEType::Custom)
        {
            ImGui::InputText("Executable", m_TempCustomPath, sizeof(m_TempCustomPath));
            ImGui::SameLine();
            if (ImGui::Button("Browse##ProjIDE"))
            {
                std::string path = FileDialog::OpenFile("Executable (*.exe)\0*.exe\0");
                if (!path.empty())
                    strncpy(m_TempCustomPath, path.c_str(), sizeof(m_TempCustomPath) - 1);
            }

            ImGui::InputText("Arguments", m_TempCustomArgs, sizeof(m_TempCustomArgs));
            ImGui::TextDisabled("Leave empty to use editor default arguments.");
            ImGui::TextDisabled("Placeholders: {ProjectDir} {Solution} {File} {ProjectName}");
        }

        ImGui::Separator();
        if (ImGui::Button("Apply"))
            ApplyToProject();

        ImGui::SameLine();
        ImGui::TextDisabled("(Use File -> Save Project to persist)");

        ImGui::End();
    }
}
