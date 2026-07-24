#pragma once

#include <filesystem>
#include <functional>
#include <string>

namespace Himii
{
    class ScriptCompiler
    {
    public:
        using CompletionCallback = std::function<void(bool success)>;

        static void RequestBuild(const std::filesystem::path& projectPath, CompletionCallback onComplete = nullptr);
        static void Update();

        static bool IsCompiling();
        static std::string GetLastLog();
        static int GetLastExitCode();

        /// 等待进行中的编译结束并终止 dotnet 子进程（编辑器退出时调用）。
        static void Shutdown();

    private:
        static void RunBuildThread(const std::filesystem::path projectPath);
    };
}
