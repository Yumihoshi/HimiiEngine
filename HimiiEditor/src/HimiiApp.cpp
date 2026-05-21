#include "EditorLayer.h"
#include "Engine.h"
#include "Himii/Core/EntryPoint.h"
#include "Himii/Renderer/RenderCommand.h"

#include <iostream>

namespace Himii
{
    class HimiiApp : public Application {
    public:
        HimiiApp(ApplicationCommandLineArgs args) : Application("Himii Editor", args)
        {
            PushOverlay(new EditorLayer());

            RenderCommand::SetClearColor(glm::vec4{0.23f, 0.23f, 0.23f, 1.0f});
            RenderCommand::Clear();
            GetWindow().Update();
        }

        virtual ~HimiiApp()
        {
            // 清理代码
        }
    };

    Application *CreateApplication(ApplicationCommandLineArgs args)
    {
        return new HimiiApp(args);
    }
} // namespace Himii
