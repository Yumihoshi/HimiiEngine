#include "Hepch.h"
#include "ScriptConsolePanel.h"
#include "Himii/Scripting/ScriptCompiler.h"
#include "Himii/Scripting/ScriptIDELauncher.h"
#include "Himii/Project/Project.h"

#include <imgui.h>
#include <regex>
#include <sstream>

namespace Himii
{
    static bool TryParseCompilerLocation(const std::string& line, std::filesystem::path& outPath, int& outLine)
    {
        static const std::regex pattern(R"((.+?)\((\d+)(?:,\d+)?\):\s*(?:error|warning))");
        std::smatch match;
        if (!std::regex_search(line, match, pattern))
            return false;

        outPath = match[1].str();
        outLine = std::stoi(match[2].str());
        return true;
    }

    void ScriptConsolePanel::OnImGuiRender(bool* pOpen)
    {
        ImGui::SetNextWindowSize(ImVec2(520, 280), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Script Console", pOpen))
        {
            ImGui::End();
            return;
        }

        if (ScriptCompiler::IsCompiling())
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Compiling...");
        else if (ScriptCompiler::GetLastExitCode() == 0)
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Last build: Success");
        else if (ScriptCompiler::GetLastExitCode() >= 0)
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Last build: Failed (code %d)", ScriptCompiler::GetLastExitCode());

        ImGui::Separator();
        ImGui::TextDisabled("Click an error line to open in IDE");

        const std::string& log = ScriptCompiler::GetLastLog();
        if (log.empty())
        {
            ImGui::TextDisabled("No build output yet.");
        }
        else
        {
            ImGui::BeginChild("ScriptLog", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            std::istringstream stream(log);
            std::string line;
            int lineIndex = 0;
            while (std::getline(stream, line))
            {
                std::filesystem::path filePath;
                int fileLine = 0;
                const bool isLocation = TryParseCompilerLocation(line, filePath, fileLine);

                if (isLocation)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.45f, 0.45f, 1.0f));

                ImGui::PushID(lineIndex++);
                if (ImGui::Selectable(line.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    if (isLocation && Project::GetActive())
                    {
                        std::filesystem::path absolutePath = filePath;
                        if (!absolutePath.is_absolute())
                        {
                            absolutePath = Project::GetProjectDirectory() / filePath;
                        }

                        ScriptIDELauncher::OpenScript(
                            Project::GetProjectDirectory(),
                            Project::GetConfig().Name,
                            absolutePath,
                            fileLine);
                    }
                }
                ImGui::PopID();

                if (isLocation)
                    ImGui::PopStyleColor();
            }

            ImGui::EndChild();
        }

        ImGui::End();
    }
}
