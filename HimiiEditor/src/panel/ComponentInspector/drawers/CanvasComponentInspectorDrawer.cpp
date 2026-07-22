#include "panel/ComponentInspector/ComponentInspectorDrawContext.h"
#include "panel/ComponentInspector/ComponentInspectorHeader.h"
#include "panel/ComponentInspector/ComponentInspectorRegistry.h"
#include "InspectorControls.h"

#include "Himii/Scene/Components.h"

#include <imgui.h>
#include <glm/glm.hpp>

namespace Himii
{
    static void DrawCanvasComponentInspectorUI(ComponentInspectorDrawContext& drawContext)
    {
        if (!drawContext.entity.HasComponent<CanvasComponent>())
            return;

        auto& component = drawContext.entity.GetComponent<CanvasComponent>();
        Ref<Texture2D> icon =
            drawContext.getComponentIcon ? drawContext.getComponentIcon("Canvas") : nullptr;

        DrawComponentInspectorHeaderUI(
            drawContext, "CanvasComponent", "Canvas", icon,
            [&]()
            {
                ImGui::TextUnformatted("Render Mode: Screen Space Overlay");
                glm::vec3 referenceResolution = {
                    component.ReferenceResolution.x, component.ReferenceResolution.y, 0.0f};
                DrawVec3Control("Reference Resolution", referenceResolution, 1920.0f);
                component.ReferenceResolution = {referenceResolution.x, referenceResolution.y};
                DrawFloatControl("Match Width Or Height", component.MatchWidthOrHeight, 0.01f);
                component.MatchWidthOrHeight = glm::clamp(component.MatchWidthOrHeight, 0.0f, 1.0f);

                if (drawContext.scene)
                    drawContext.scene->SyncCanvasReferenceResolutionToTransform(drawContext.entity);
            },
            [&]() { drawContext.entity.RemoveComponent<CanvasComponent>(); });
    }

    struct CanvasComponentInspectorRegistrar
    {
        CanvasComponentInspectorRegistrar()
        {
            ComponentInspectorRegistry::Get().RegisterComponentInspector<CanvasComponent>(
                "CanvasComponent", "Canvas", "Canvas", 190, &DrawCanvasComponentInspectorUI);
        }
    };

    static CanvasComponentInspectorRegistrar s_CanvasComponentInspectorRegistrar;
}
