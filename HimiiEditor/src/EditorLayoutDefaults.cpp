#include "EditorLayoutDefaults.h"

#include "Himii/Core/Application.h"
#include "imgui_internal.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace Himii
{
    namespace EditorLayoutDefaults
    {
        std::filesystem::path GetEditorLayoutIniPath()
        {
            return Application::Get().GetExecutableDir() / "editor_layout.ini";
        }

        static bool IniFileContainsDockingSection(const std::filesystem::path &layout_ini_path)
        {
            std::ifstream input(layout_ini_path);
            if (!input.is_open())
                return false;

            std::stringstream buffer;
            buffer << input.rdbuf();
            return buffer.str().find("[Docking]") != std::string::npos;
        }

        static bool ShouldBuildDefaultLayout()
        {
            const std::filesystem::path layout_ini_path = GetEditorLayoutIniPath();
            if (!std::filesystem::exists(layout_ini_path))
                return true;

            // 仅有 [Window] 浮动坐标、没有 [Docking] 时仍会乱布局，需要重建默认停靠
            return !IniFileContainsDockingSection(layout_ini_path);
        }

        void ApplyDefaultDockLayoutIfNeeded(ImGuiID dockspace_id)
        {
            static bool default_layout_built = false;
            if (default_layout_built)
                return;

            if (!ShouldBuildDefaultLayout())
                return;

            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            const ImVec2 dockspace_size = ImGui::GetWindowSize();
            ImGui::DockBuilderSetNodeSize(dockspace_id,
                                          ImVec2(dockspace_size.x > 0.0f ? dockspace_size.x : ImGui::GetMainViewport()->Size.x,
                                                 dockspace_size.y > 0.0f ? dockspace_size.y : ImGui::GetMainViewport()->Size.y));

            ImGuiID dock_main_id = dockspace_id;

            ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.28f, nullptr,
                                                                &dock_main_id);
            ImGuiID dock_id_left = dock_main_id;

            ImGuiID dock_id_left_bottom = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.30f, nullptr,
                                                                      &dock_id_left);
            ImGuiID dock_id_console_area = ImGui::DockBuilderSplitNode(dock_id_left_bottom, ImGuiDir_Right, 0.27f,
                                                                       nullptr, &dock_id_left_bottom);

            ImGuiID dock_id_right_bottom = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.70f, nullptr,
                                                                       &dock_id_right);

            ImGuiID dock_id_toolbar = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Up, 0.045f, nullptr,
                                                                  &dock_id_left);

            ImGui::DockBuilderDockWindow("##toolbar", dock_id_toolbar);
            ImGui::DockBuilderDockWindow("ViewPort", dock_id_left);
            ImGui::DockBuilderDockWindow("Content Browser", dock_id_left_bottom);
            ImGui::DockBuilderDockWindow("Console", dock_id_console_area);
            ImGui::DockBuilderDockWindow("Script Console", dock_id_console_area);
            ImGui::DockBuilderDockWindow("Stats", dock_id_console_area);
            ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_id_right);
            ImGui::DockBuilderDockWindow("Properties", dock_id_right_bottom);

            ImGui::DockBuilderFinish(dockspace_id);

            if (ImGuiDockNode *toolbar_dock_node = ImGui::DockBuilderGetNode(dock_id_toolbar))
            {
                toolbar_dock_node->SetLocalFlags(toolbar_dock_node->LocalFlags |
                                                 ImGuiDockNodeFlags_HiddenTabBar);
            }

            default_layout_built = true;
        }
    }
}
