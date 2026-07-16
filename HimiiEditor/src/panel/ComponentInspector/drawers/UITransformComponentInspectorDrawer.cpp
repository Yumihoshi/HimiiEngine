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

namespace Himii
{
    static void DrawUITransformComponentInspectorUI(ComponentInspectorDrawContext& drawContext)
    {
        if (!drawContext.entity.HasComponent<UITransformComponent>())
            return;

        auto& component = drawContext.entity.GetComponent<UITransformComponent>();
        Ref<Texture2D> icon =
            drawContext.getComponentIcon ? drawContext.getComponentIcon("Rect Transform") : nullptr;

        DrawComponentInspectorHeaderUI(
            drawContext, "UITransformComponent", "Rect Transform", icon,
            [&]()
            {
                UITransformComponent transformBeforeEdit = component;
                bool transformEditCaptured = false;

                auto beginTransformEdit = [&]()
                {
                    if (!transformEditCaptured)
                    {
                        transformBeforeEdit = component;
                        transformEditCaptured = true;
                    }
                };

                auto endTransformEdit = [&]()
                {
                    if (!transformEditCaptured || drawContext.commandHistory == nullptr)
                        return;

                    UITransformComponent transformAfterEdit = component;
                    if (transformBeforeEdit.Position != transformAfterEdit.Position
                        || transformBeforeEdit.Rotation != transformAfterEdit.Rotation
                        || transformBeforeEdit.Size != transformAfterEdit.Size)
                    {
                        drawContext.commandHistory->Execute(std::make_unique<ModifyUITransformCommand>(
                            drawContext.scene,
                            drawContext.entity.GetUUID(),
                            transformBeforeEdit,
                            transformAfterEdit));
                    }
                    transformEditCaptured = false;
                };

                DrawVec3Control("Rect Position", component.Position, 0.0f, beginTransformEdit, endTransformEdit);

                glm::vec3 rotationDegrees = glm::degrees(component.Rotation);
                DrawVec3Control("Rotation", rotationDegrees, 0.0f, beginTransformEdit, endTransformEdit);
                component.Rotation = glm::radians(rotationDegrees);

                glm::vec3 sizeVector = glm::vec3(component.Size, 1.0f);
                DrawVec3Control("Size", sizeVector, 100.0f, beginTransformEdit, endTransformEdit);
                component.Size = glm::vec2(sizeVector.x, sizeVector.y);
            },
            [&]() { drawContext.entity.RemoveComponent<UITransformComponent>(); });
    }

    struct UITransformComponentInspectorRegistrar
    {
        UITransformComponentInspectorRegistrar()
        {
            ComponentInspectorRegistry::Get().RegisterComponentInspector<UITransformComponent>(
                "UITransformComponent", "Rect Transform", "Rect Transform", 200,
                &DrawUITransformComponentInspectorUI);
        }
    };

    static UITransformComponentInspectorRegistrar s_UITransformComponentInspectorRegistrar;
}
