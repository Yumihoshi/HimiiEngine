#pragma once
#include "Himii/Renderer/Texture.h"

#include <filesystem>
#include <functional>

namespace Himii
{
    class ContentBrowserPanel {
    public:
        ContentBrowserPanel();

        void OnImGuiRender();

        void Refresh();
        void SetOnScriptChanged(std::function<void()> callback) { m_OnScriptChanged = std::move(callback); }
    private:
        bool CreateCSharpScript(const std::filesystem::path& directory, const std::string& className);
        std::filesystem::path m_BaseDirectory;
        std::filesystem::path m_CurrentDirectory;

        Ref<Texture2D> m_DirectoryIcon;
        Ref<Texture2D> m_FileIcon;
        Ref<Texture2D> m_ScriptIcon;
        Ref<Texture2D> m_SceneIcon;
        
        void DrawTree(const std::filesystem::path& path);

        std::function<void()> m_OnScriptChanged;
    };
}
