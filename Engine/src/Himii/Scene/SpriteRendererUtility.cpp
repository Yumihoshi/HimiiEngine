#include "Hepch.h"
#include "Himii/Scene/SpriteRendererUtility.h"

#include "Himii/Asset/AssetManager.h"
#include "Himii/Asset/SpriteSheetUtility.h"
#include "Himii/Scene/Entity.h"
#include "Himii/Scene/SpriteAnimation.h"

namespace Himii
{

    SpriteResolved ResolveSpriteRendererDrawable(Entity entity,
                                                 const SpriteRendererComponent& spriteRenderer,
                                                 AssetManager* assetManager)
    {
        if (!assetManager)
            return {};

        if (entity && entity.HasComponent<SpriteAnimationComponent>())
        {
            const SpriteAnimationComponent& animationComponent =
                entity.GetComponent<SpriteAnimationComponent>();

            if (animationComponent.Playing && animationComponent.AnimationHandle != 0
                && assetManager->IsAssetHandleValid(animationComponent.AnimationHandle))
            {
                Ref<SpriteAnimation> animation = std::static_pointer_cast<SpriteAnimation>(
                    assetManager->GetAsset(animationComponent.AnimationHandle));

                if (animation && animation->GetFrameCount() > 0 && animation->UsesAtlasFrames())
                {
                    const glm::ivec2 atlasCoordinates =
                        animation->GetAtlasFrameCoordinates(animationComponent.CurrentFrame);
                    const AssetHandle atlasTextureHandle = animation->GetAtlasTextureHandle();
                    const uint32_t gridCellSize = animation->GetAtlasGridCellSize();

                    Ref<Asset> atlasAsset = assetManager->GetAsset(atlasTextureHandle);
                    if (!atlasAsset)
                        return {};

                    Ref<Texture2D> atlasTexture = std::static_pointer_cast<Texture2D>(atlasAsset);

                    SpriteResolved resolved;
                    resolved.Texture = atlasTexture;
                    resolved.UVs = SpriteSheetUtility::AtlasGridCoordsToUVs(
                        atlasCoordinates, gridCellSize,
                        atlasTexture->GetWidth(), atlasTexture->GetHeight());
                    resolved.Pivot = {0.5f, 0.5f};
                    resolved.PixelSize = {static_cast<int>(gridCellSize), static_cast<int>(gridCellSize)};

                    const TextureImportData* importData =
                        assetManager->GetTextureImportData(atlasTextureHandle);
                    resolved.PixelsPerUnit =
                        importData && importData->PixelsPerUnit > 0 ? importData->PixelsPerUnit : 100;
                    resolved.IsValid = true;
                    return resolved;
                }
            }
        }

        if (spriteRenderer.SpriteAssetHandle != 0)
            return assetManager->ResolveSprite(spriteRenderer.SpriteAssetHandle);

        return {};
    }

    glm::mat4 GetSpriteRendererVisualTransform(const TransformComponent& transform,
                                               const SpriteResolved& resolved)
    {
        if (!resolved.IsValid)
            return transform.GetTransform();

        return SpriteSheetUtility::BuildSpriteRenderTransform(
            transform.GetTransform(), resolved.PixelSize, resolved.PixelsPerUnit, resolved.Pivot);
    }

} // namespace Himii
