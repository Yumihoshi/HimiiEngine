#pragma once

#include <filesystem>
#include <vector>

struct GLFWwindow;

namespace Himii
{
    /// 接收操作系统拖入 GLFW 窗口的文件路径（主线程消费）
    class EditorExternalFileDrop
    {
    public:
        static void Install(GLFWwindow* window);
        static std::vector<std::filesystem::path> ConsumePendingPaths();
    };
}
