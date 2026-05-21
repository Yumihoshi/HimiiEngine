#pragma once

#include "imgui.h"

#include <filesystem>

namespace Himii
{
    /// 编辑器首次启动（无本地 editor_layout.ini）时，用 DockBuilder 建立默认停靠布局。
    namespace EditorLayoutDefaults
    {
        std::filesystem::path GetEditorLayoutIniPath();

        /// 在 ImGui::DockSpace 之后调用；若用户已有含 [Docking] 的 layout ini 则不做任何事。
        void ApplyDefaultDockLayoutIfNeeded(ImGuiID dockspace_id);
    }
}
