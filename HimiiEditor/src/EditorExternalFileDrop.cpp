#include "EditorExternalFileDrop.h"
#include <GLFW/glfw3.h>

namespace Himii
{
    static std::vector<std::filesystem::path> s_PendingFilePaths;

    void EditorExternalFileDrop::Install(GLFWwindow* window)
    {
        if (!window)
            return;

        glfwSetDropCallback(window, [](GLFWwindow*, int pathCount, const char** paths)
        {
            s_PendingFilePaths.clear();
            for (int index = 0; index < pathCount; ++index)
            {
                if (paths[index])
                    s_PendingFilePaths.emplace_back(paths[index]);
            }
        });
    }

    std::vector<std::filesystem::path> EditorExternalFileDrop::ConsumePendingPaths()
    {
        std::vector<std::filesystem::path> result = std::move(s_PendingFilePaths);
        s_PendingFilePaths.clear();
        return result;
    }
}
