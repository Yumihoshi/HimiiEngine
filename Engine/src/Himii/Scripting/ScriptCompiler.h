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
        static const std::string& GetLastLog();
        static int GetLastExitCode();

    private:
        static void RunBuildThread(const std::filesystem::path projectPath);
    };
}
