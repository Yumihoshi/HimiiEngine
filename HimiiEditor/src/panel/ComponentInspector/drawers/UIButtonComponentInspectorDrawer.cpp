#include "panel/ComponentInspector/ComponentInspectorDrawContext.h"
#include "panel/ComponentInspector/ComponentInspectorHeader.h"
#include "panel/ComponentInspector/ComponentInspectorRegistry.h"
#include "panel/ComponentInspector/ComponentInspectorUtils.h"
#include "InspectorControls.h"

#include "Himii/Scene/Components.h"

#include <imgui.h>

namespace Himii
{
    static void DrawUIButtonComponentInspectorUI(ComponentInspectorDrawContext& drawContext)
    {
        if (!drawContext.entity.HasComponent<UIButtonComponent>())
            return;

        auto& component = drawContext.entity.GetComponent<UIButtonComponent>();
        Ref<Texture2D> icon = drawContext.getComponentIcon ? drawContext.getComponentIcon("Button") : nullptr;

        DrawComponentInspectorHeaderUI(
            drawContext, "UIButtonComponent", "Button", icon,
            [&]()
            {
                ImGui::Checkbox("Interactable", &component.Interactable);
                DrawColorControl("Normal", component.Colors.NormalColor);
                DrawColorControl("Highlighted", component.Colors.HighlightedColor);
                DrawColorControl("Pressed", component.Colors.PressedColor);
                DrawColorControl("Disabled", component.Colors.DisabledColor);

                if (!drawContext.entity.HasComponent<UIImageComponent>())
                    ImGui::TextDisabled("No Image: acts as an invisible hit area.");
            },
            [&]() { drawContext.entity.RemoveComponent<UIButtonComponent>(); });
    }

    struct UIButtonComponentInspectorRegistrar
    {
        UIButtonComponentInspectorRegistrar()
        {
            ComponentInspectorRegistry::Get().RegisterComponentInspector<UIButtonComponent>(
                "UIButtonComponent", "Button", "Button", 220, &DrawUIButtonComponentInspectorUI);
        }
    };

    static UIButtonComponentInspectorRegistrar s_UIButtonComponentInspectorRegistrar;
}
