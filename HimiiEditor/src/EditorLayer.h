#pragma once
#include "Engine.h"
#include "panel/SceneHierarchyPanel.h"
#include "panel/ContentBrowserPanel.h"
#include "panel/AnimationPanel.h"
#include "panel/TileMapEditorPanel.h"
#include "panel/ParticleEmitterEditorPanel.h"
#include "panel/ScriptConsolePanel.h"
#include "panel/ConsolePanel.h"
#include "panel/EditorPreferencesPanel.h"
#include "panel/ProjectSettingsPanel.h"

#include "Himii/Core/FileWatcher.h"
#include "Himii/Renderer/EditorCamera.h"

namespace Himii
{
    class EditorLayer : public Himii::Layer {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnUpdate(Himii::Timestep ts) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Himii::Event &event) override;

    private:
        bool OnKeyPressed(KeyPressedEvent &e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent &e);

        void OnOverlayRender();

        void CreateProject(const std::filesystem::path& projectPath, bool is2D = true);
        void OpenProject(const std::filesystem::path &path);
        void SaveProject();
        void BuildProject();

        void OpenCSProject();

        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveScene();
        void SaveSceneAs();

        void SerializeScene(Ref<Scene> scene, const std::filesystem::path &path);

        void OnScenePlay();
        void OnSceneSimulate();
        void OnSceneStop();

        void OnDuplicateEntity();
        void CompileAndReloadScripts();
        void RequestScriptCompile();
        void StartScenePlay();

        //UI panel
        void UI_Toolbar();
    private:
        Ref<Scene> m_ActiveScene;
        Ref<Scene> m_EditorScene;

        std::filesystem::path m_EditorScenePath;
        std::filesystem::path m_CSharpProjectPath;

        Entity m_SquareEntity;
        Entity m_CameraEntity;

        Entity m_HoveredEntity;

        OrthographicCameraController m_CameraController;
        EditorCamera m_EditorCamera;

        bool m_ViewportFocused = false, m_ViewportHovered = false;

        // Scene 视口用的离屏帧缓冲
        Ref<Framebuffer> m_Framebuffer;

        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
        glm::vec2 m_ViewportBounds[2];

        int m_GizmoType = -1;

        bool m_ShowPhysicsColliders = false;
        bool m_ShowGrid = true;

        enum class SceneState {
            Edit = 0,
            Play = 1,
            Simulate = 2
        };
        SceneHierarchyPanel m_SceneHierarchyPanel;
        ContentBrowserPanel m_ContentBrowserPanel;

        AnimationPanel m_AnimationPanel;
        bool m_ShowAnimationPanel = false;

        TileMapEditorPanel m_TileMapEditorPanel;
        bool m_ShowTileMapEditor = false;

        ParticleEmitterEditorPanel m_ParticleEmitterEditorPanel;
        bool m_ShowParticleEmitterEditor = false;

        ScriptConsolePanel m_ScriptConsolePanel;
        bool m_ShowScriptConsole = true;

        ConsolePanel m_ConsolePanel;
        bool m_ShowConsole = true;

        EditorPreferencesPanel m_EditorPreferencesPanel;
        bool m_ShowEditorPreferences = false;

        ProjectSettingsPanel m_ProjectSettingsPanel;
        bool m_ShowProjectSettings = false;

        FileWatcher m_ScriptFileWatcher;
        bool m_ScriptsDirty = false;
        bool m_PendingPlayAfterCompile = false;
        bool m_ShowScriptReloadNotice = false;
        bool m_WasScriptCompiling = false;
        bool m_NotifyReloadAfterCompile = false;

        Ref<Texture2D> m_IconPlay, m_IconStop, m_IconSimulate;
        Ref<TextureCube> m_SkyboxTexture;

        SceneState m_SceneState = SceneState::Edit;
        std::string m_Clipboard;

        Ref<Font> m_DefaultFont;

    private:
        struct RecentProject
        {
            std::string Name;
            std::string FilePath;
            long long LastOpened;
        };
        std::vector<RecentProject> m_RecentProjects;

        void LoadRecentProjects();
        void SaveRecentProjects();
        void DrawProjectHub();
    };
}