#pragma once

#include "Himii/Asset/Sprite.h"
#include "Himii/Scene/Components.h"

namespace Himii
{

    class AssetManager;
    class Entity;

    SpriteResolved ResolveSpriteRendererDrawable(Entity entity,
                                                 const SpriteRendererComponent& spriteRenderer,
                                                 AssetManager* assetManager);

    glm::mat4 GetSpriteRendererVisualTransform(const TransformComponent& transform,
                                               const SpriteResolved& resolved);

} // namespace Himii
