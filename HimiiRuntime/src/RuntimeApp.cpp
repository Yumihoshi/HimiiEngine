#include "Engine.h"
#include "Himii/Core/EntryPoint.h"
#include "Himii/Core/Input.h"
#include "Himii/Project/Project.h"
#include "Himii/Scripting/ScriptEngine.h"

namespace Himii
{
    class RuntimeLayer : public Himii::Layer {
    public:
        RuntimeLayer() : Layer("RuntimeLayer")
        {
        }

        void OnAttach() override
        {
            std::filesystem::path projectFile;
            bool foundProject = false;

            for (auto &entry: std::filesystem::directory_iterator(std::filesystem::current_path()))
            {
                if (entry.path().extension() == ".hproj")
                {
                    projectFile = entry.path();
                    foundProject = true;
                    break;
                }
            }

            if (!foundProject)
            {
                HIMII_CORE_ERROR("No .hproj file found in current directory!");
                return;
            }

            HIMII_CORE_INFO("Loading Project: {0}", projectFile.string());
            Ref<Project> project = Project::Load(projectFile);

            std::filesystem::path gameDllPath =
                    std::filesystem::absolute(Project::GetProjectDirectory() / Project::GetConfig().ScriptModulePath);
            ScriptEngine::LoadAppAssembly(gameDllPath);

            std::filesystem::path startScenePath = Project::GetAssetDirectory() / Project::GetConfig().StartScene;

            if (std::filesystem::exists(startScenePath))
            {
                HIMII_CORE_INFO("Loading Start Scene: {0}", startScenePath.string());
                Ref<Scene> newScene = CreateRef<Scene>();
                SceneSerializer serializer(newScene);
                if (serializer.Deserialize(startScenePath.string()))
                {
                    m_ActiveScene = newScene;
                    m_ActiveScene->OnRuntimeStart();

                    auto &window = Application::Get().GetWindow();
                    m_ActiveScene->OnViewportResize(window.GetWidth(), window.GetHeight());
                }
            }
            else
            {
                HIMII_CORE_ERROR("Start scene not found!");
            }
        }

        void OnUpdate(Timestep ts) override
        {
            if (!m_ActiveScene)
                return;

            auto& window = Application::Get().GetWindow();
            m_ActiveScene->OnViewportResize(window.GetWidth(), window.GetHeight());

            Scene::UserInterfacePointerFrameInput userInterfacePointerInput{};
            userInterfacePointerInput.Enabled = true;
            userInterfacePointerInput.HasPosition = true;
            const glm::vec2 mousePosition = Input::GetMousePosition();
            // 窗口坐标 Y 向下；UI ortho Y 向上。
            userInterfacePointerInput.PositionInTargetPixels = {
                    mousePosition.x,
                    static_cast<float>(window.GetHeight()) - mousePosition.y};
            const bool primaryButtonHeld = Input::IsMouseButtonPressed(Mouse::ButtonLeft);
            userInterfacePointerInput.PrimaryButtonHeld = primaryButtonHeld;
            userInterfacePointerInput.PrimaryButtonPressedThisFrame =
                    primaryButtonHeld && !m_PrimaryButtonWasHeld;
            userInterfacePointerInput.PrimaryButtonReleasedThisFrame =
                    !primaryButtonHeld && m_PrimaryButtonWasHeld;
            m_PrimaryButtonWasHeld = primaryButtonHeld;
            m_ActiveScene->SetUserInterfacePointerInput(userInterfacePointerInput);

            RenderCommand::SetClearColor({0.1f, 0.12f, 0.16f, 1.0f});
            RenderCommand::Clear();
            m_ActiveScene->OnUpdateRuntime(ts);
        }

        private:
        Ref<Scene> m_ActiveScene;
        bool m_PrimaryButtonWasHeld = false;
    };

    class Runtime : public Application {
    public:
        Runtime(const ApplicationCommandLineArgs &args) : Application("Himii Game Engine", args)
        {
            PushLayer(new RuntimeLayer());
        }

        ~Runtime()
        {
        }
    };

    Application *CreateApplication(ApplicationCommandLineArgs args)
    {
        return new Runtime(args);
    }
} // namespace Himii
