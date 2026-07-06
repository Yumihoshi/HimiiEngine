#include "Hepch.h"
#include "ScriptConsolePanel.h"
#include "Himii/Scripting/ScriptCompiler.h"
#include "Himii/Scripting/ScriptIDELauncher.h"
#include "Himii/Project/Project.h"

#include <imgui.h>
#include <sstream>

namespace Himii
{
    static bool TryParseCompilerLocation(const std::string& line, std::filesystem::path& outPath, int& outLine)
    {
        const size_t messagePosition = [&]() -> size_t
        {
            const size_t errorPosition = line.find(": error");
            if (errorPosition != std::string::npos)
                return errorPosition;

            const size_t warningPosition = line.find(": warning");
            if (warningPosition != std::string::npos)
                return warningPosition;

            return std::string::npos;
        }();

        if (messagePosition == std::string::npos)
            return false;

        const size_t closeParenthesis = line.rfind(')', messagePosition);
        if (closeParenthesis == std::string::npos)
            return false;

        const size_t openParenthesis = line.rfind('(', closeParenthesis);
        if (openParenthesis == std::string::npos || openParenthesis >= closeParenthesis)
            return false;

        std::string lineNumberText = line.substr(openParenthesis + 1, closeParenthesis - openParenthesis - 1);
        const size_t commaPosition = lineNumberText.find(',');
        if (commaPosition != std::string::npos)
            lineNumberText = lineNumberText.substr(0, commaPosition);

        if (lineNumberText.empty())
            return false;

        try
        {
            outLine = std::stoi(lineNumberText);
        }
        catch (...)
        {
            return false;
        }

        outPath = line.substr(0, openParenthesis);
        std::string pathText = outPath.string();
        while (!pathText.empty() && (pathText.front() == ' ' || pathText.front() == '\t'))
            pathText.erase(pathText.begin());

        if (pathText.empty())
            return false;

        outPath = pathText;
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
