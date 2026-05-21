#include "EditorLayer.h"
#include "Engine.h"
#include "Himii/Core/EntryPoint.h"

#include <iostream>

namespace Himii
{
    class HimiiApp : public Application {
    public:
        HimiiApp(ApplicationCommandLineArgs args)
            : Application("Himii Editor", args, CreateStartupWindowProps(), true)
        {
            PushOverlay(new EditorLayer());
        }

        virtual ~HimiiApp()
        {
        }

    private:
        static WindowProps CreateStartupWindowProps()
        {
            WindowProps startup_window_props("Himii Editor", 128, 128);
            startup_window_props.Decorated = false;
            startup_window_props.TransparentFramebuffer = true;
            startup_window_props.Maximized = false;
            startup_window_props.CenterOnScreen = true;
            return startup_window_props;
        }
    };

    Application *CreateApplication(ApplicationCommandLineArgs args)
    {
        return new HimiiApp(args);
    }
} // namespace Himii
