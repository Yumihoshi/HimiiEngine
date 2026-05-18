#include "Hepch.h"
#include "ScriptCompiler.h"

#include <atomic>
#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Himii
{
    namespace
    {
        std::mutex s_LogMutex;
        std::string s_LastLog;
        std::atomic<bool> s_IsCompiling{false};
        std::atomic<int> s_LastExitCode{-1};
        std::atomic<bool> s_HasPendingResult{false};
        std::atomic<bool> s_PendingSuccess{false};
        ScriptCompiler::CompletionCallback s_PendingCallback;
        std::mutex s_CallbackMutex;
    }

    void ScriptCompiler::RequestBuild(const std::filesystem::path& projectPath, CompletionCallback onComplete)
    {
        if (s_IsCompiling.load())
            return;

        {
            std::lock_guard<std::mutex> lock(s_CallbackMutex);
            s_PendingCallback = std::move(onComplete);
        }

        s_IsCompiling.store(true);
        std::thread(RunBuildThread, projectPath).detach();
    }

    void ScriptCompiler::RunBuildThread(const std::filesystem::path projectPath)
    {
        std::ostringstream command;
        command << "dotnet build \"" << projectPath.string() << "\" -c Debug -v minimal 2>&1";

        std::string output;
        int exitCode = -1;

#ifdef _WIN32
        FILE* pipe = _popen(command.str().c_str(), "r");
        if (pipe)
        {
            char buffer[512];
            while (fgets(buffer, sizeof(buffer), pipe))
                output += buffer;
            exitCode = _pclose(pipe);
        }
        else
        {
            output = "[ScriptCompiler] Failed to start dotnet build process.\n";
        }
#else
        FILE* pipe = popen(command.str().c_str(), "r");
        if (pipe)
        {
            char buffer[512];
            while (fgets(buffer, sizeof(buffer), pipe))
                output += buffer;
            exitCode = pclose(pipe);
        }
        else
        {
            output = "[ScriptCompiler] Failed to start dotnet build process.\n";
        }
#endif

        {
            std::lock_guard<std::mutex> lock(s_LogMutex);
            s_LastLog = std::move(output);
            s_LastExitCode.store(exitCode);
        }

        s_PendingSuccess.store(exitCode == 0);
        s_HasPendingResult.store(true);
        s_IsCompiling.store(false);
    }

    void ScriptCompiler::Update()
    {
        if (!s_HasPendingResult.exchange(false))
            return;

        CompletionCallback callback;
        {
            std::lock_guard<std::mutex> lock(s_CallbackMutex);
            callback = std::move(s_PendingCallback);
            s_PendingCallback = nullptr;
        }

        if (callback)
            callback(s_PendingSuccess.load());
    }

    bool ScriptCompiler::IsCompiling()
    {
        return s_IsCompiling.load();
    }

    const std::string& ScriptCompiler::GetLastLog()
    {
        std::lock_guard<std::mutex> lock(s_LogMutex);
        return s_LastLog;
    }

    int ScriptCompiler::GetLastExitCode()
    {
        return s_LastExitCode.load();
    }
}
