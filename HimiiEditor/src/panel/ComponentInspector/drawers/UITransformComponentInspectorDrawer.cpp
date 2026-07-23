#include "panel/ComponentInspector/ComponentInspectorDrawContext.h"
#include "panel/ComponentInspector/ComponentInspectorHeader.h"
#include "panel/ComponentInspector/ComponentInspectorRegistry.h"
#include "panel/ComponentInspector/ComponentInspectorUtils.h"
#include "InspectorControls.h"
#include "commands/EditorCommandHistory.h"
#include "commands/EditorCommands.h"

#include "Himii/Renderer/Font.h"
#include "Himii/Scene/Components.h"

#include <glm/gtc/constants.hpp>
#include <imgui.h>

namespace Himii
{
    static void DrawRectTransformComponentInspectorUserInterface(
            ComponentInspectorDrawContext& drawContext)
    {
        if (!drawContext.entity.HasComponent<RectTransformComponent>())
            return;

        auto& component = drawContext.entity.GetComponent<RectTransformComponent>();
        Ref<Texture2D> icon =
            drawContext.getComponentIcon ? drawContext.getComponentIcon("Rect Transform") : nullptr;

        DrawComponentInspectorHeaderUI(
            drawContext, "RectTransformComponent", "Rect Transform", icon,
            [&]()
            {
                if (drawContext.entity.HasComponent<CanvasComponent>())
                {
                    ImGui::TextDisabled("Canvas root is resolved from the Game target size.");
                    if (drawContext.scene)
                        drawContext.scene->SyncCanvasReferenceResolutionToTransform(drawContext.entity);
                    ImGui::Text(
                            "Resolved Size: %.0f x %.0f",
                            component.ResolvedSize.x, component.ResolvedSize.y);
                    return;
                }

                RectTransformComponent transformBeforeEdit = component;
                bool transformEditCaptured = false;

                auto beginTransformEdit = [&]()
                {
                    if (!transformEditCaptured)
                    {
                        transformBeforeEdit = component;
                        transformEditCaptured = true;
                    }
                };

                auto notifyTransformPreview = [&]()
                {
                    if (drawContext.scene)
                        drawContext.scene->MarkEntityTransformDirty(drawContext.entity);
                };

                auto endTransformEdit = [&]()
                {
                    notifyTransformPreview();

                    if (!transformEditCaptured || drawContext.commandHistory == nullptr)
                    {
                        transformEditCaptured = false;
                        return;
                    }

                    RectTransformComponent transformAfterEdit = component;
                    if (transformBeforeEdit.AnchorMinimum != transformAfterEdit.AnchorMinimum
                        || transformBeforeEdit.AnchorMaximum != transformAfterEdit.AnchorMaximum
                        || transformBeforeEdit.Pivot != transformAfterEdit.Pivot
                        || transformBeforeEdit.AnchoredPosition != transformAfterEdit.AnchoredPosition
                        || transformBeforeEdit.RotationRadians
                                != transformAfterEdit.RotationRadians
                        || transformBeforeEdit.SizeDelta != transformAfterEdit.SizeDelta)
                    {
                        drawContext.commandHistory->Execute(std::make_unique<ModifyRectTransformCommand>(
                            drawContext.scene,
                            drawContext.entity.GetUUID(),
                            transformBeforeEdit,
                            transformAfterEdit));
                    }
                    transformEditCaptured = false;
                };

                DrawVec2AxisControl(
                        "Anchor Minimum", component.AnchorMinimum, 0.5f,
                        beginTransformEdit, endTransformEdit);
                component.AnchorMinimum = glm::clamp(
                        component.AnchorMinimum, glm::vec2(0.0f), glm::vec2(1.0f));

                DrawVec2AxisControl(
                        "Anchor Maximum", component.AnchorMaximum, 0.5f,
                        beginTransformEdit, endTransformEdit);
                component.AnchorMaximum = glm::clamp(
                        component.AnchorMaximum, component.AnchorMinimum, glm::vec2(1.0f));

                DrawVec2AxisControl(
                        "Pivot", component.Pivot, 0.5f,
                        beginTransformEdit, endTransformEdit);
                component.Pivot = glm::clamp(
                        component.Pivot, glm::vec2(0.0f), glm::vec2(1.0f));

                DrawVec2AxisControl(
                        "Anchored Position", component.AnchoredPosition, 0.0f,
                        beginTransformEdit, endTransformEdit);

                float rotationDegrees = glm::degrees(component.RotationRadians);
                DrawPropertyRow(
                        "Rotation",
                        [&]()
                        {
                            ImGui::PushItemWidth(-1.0f);
                            ImGui::DragFloat(
                                    "##RotationDegrees", &rotationDegrees,
                                    0.1f, 0.0f, 0.0f, "%.2f deg");
                            ImGui::PopItemWidth();
                            component.RotationRadians = glm::radians(rotationDegrees);
                            if (ImGui::IsItemActivated())
                                beginTransformEdit();
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                endTransformEdit();
                        });

                DrawVec2AxisControl(
                        "Size Delta", component.SizeDelta, 100.0f,
                        beginTransformEdit, endTransformEdit);

                if (transformEditCaptured)
                    notifyTransformPreview();
            },
            [&]() { drawContext.entity.RemoveComponent<RectTransformComponent>(); });
    }

    struct RectTransformComponentInspectorRegistrar
    {
        RectTransformComponentInspectorRegistrar()
        {
            ComponentInspectorRegistry::Get().RegisterComponentInspector<RectTransformComponent>(
                "RectTransformComponent", "Rect Transform", "Rect Transform", 200,
                &DrawRectTransformComponentInspectorUserInterface);
        }
    };

    static RectTransformComponentInspectorRegistrar
            s_RectTransformComponentInspectorRegistrar;
}
