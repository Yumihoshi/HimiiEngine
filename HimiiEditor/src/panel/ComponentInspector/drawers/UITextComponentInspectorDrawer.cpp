#include "panel/ComponentInspector/ComponentInspectorDrawContext.h"
#include "panel/ComponentInspector/ComponentInspectorHeader.h"
#include "panel/ComponentInspector/ComponentInspectorRegistry.h"
#include "InspectorControls.h"

#include "Himii/Renderer/Font.h"
#include "Himii/Scene/Components.h"

#include <imgui.h>

namespace Himii
{
    static void DrawUITextComponentInspectorUI(ComponentInspectorDrawContext& drawContext)
    {
        if (!drawContext.entity.HasComponent<UITextComponent>())
            return;

        auto& component = drawContext.entity.GetComponent<UITextComponent>();
        Ref<Texture2D> icon = drawContext.getComponentIcon ? drawContext.getComponentIcon("Text") : nullptr;

        DrawComponentInspectorHeaderUI(
            drawContext, "UITextComponent", "Text", icon,
            [&]()
            {
                ImGui::Text("Content");
                ImGui::PushID("Content");
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                snprintf(buffer, sizeof(buffer), "%s", component.TextString.c_str());
                if (ImGui::InputTextMultiline("##TextContent", buffer, sizeof(buffer),
                                              ImVec2(-1, ImGui::GetFontSize() * 3)))
                {
                    component.TextString = std::string(buffer);
                }
                ImGui::PopID();

                DrawColorControl("Text Color", component.Color);
                DrawFloatControl("Font Size", component.FontSize, 1.0f);
                if (component.FontSize < 1.0f)
                    component.FontSize = 1.0f;
                DrawFloatControl("Kerning", component.Kerning, 0.01f);
                DrawFloatControl("Line Spacing", component.LineSpacing, 0.01f);

                static const char* horizontalAlignmentLabels[] = {
                        "Left", "Center", "Right"};
                int horizontalAlignmentIndex =
                        static_cast<int>(component.HorizontalAlignment);
                DrawEnumComboControl(
                        "Horizontal Alignment", horizontalAlignmentIndex,
                        horizontalAlignmentLabels, 3,
                        [&](int selectedIndex)
                        {
                            component.HorizontalAlignment =
                                    static_cast<TextHorizontalAlignment>(selectedIndex);
                        });

                static const char* verticalAlignmentLabels[] = {
                        "Top", "Middle", "Bottom"};
                int verticalAlignmentIndex =
                        static_cast<int>(component.VerticalAlignment);
                DrawEnumComboControl(
                        "Vertical Alignment", verticalAlignmentIndex,
                        verticalAlignmentLabels, 3,
                        [&](int selectedIndex)
                        {
                            component.VerticalAlignment =
                                    static_cast<TextVerticalAlignment>(selectedIndex);
                        });

                ImGui::Text("Font: %s", component.FontAsset ? "Assigned" : "None (Default)");
                if (!component.FontAsset)
                {
                    if (ImGui::Button("Attach Default Font"))
                        component.FontAsset = Font::GetDefault();
                }
            },
            [&]() { drawContext.entity.RemoveComponent<UITextComponent>(); });
    }

    struct UITextComponentInspectorRegistrar
    {
        UITextComponentInspectorRegistrar()
        {
            ComponentInspectorRegistry::Get().RegisterComponentInspector<UITextComponent>(
                "UITextComponent", "Text", "Text", 220, &DrawUITextComponentInspectorUI);
        }
    };

    static UITextComponentInspectorRegistrar s_UITextComponentInspectorRegistrar;
}
