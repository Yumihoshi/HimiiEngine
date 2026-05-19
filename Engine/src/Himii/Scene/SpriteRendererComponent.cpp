#include "Himii/Scene/Components.h"
#include "Himii/Asset/AssetManager.h"
#include "Himii/Asset/SpriteSheetUtility.h"

namespace Himii
{

    void SpriteRendererComponent::ResolveResources(AssetManager* assetManager)
    {
        UseSpriteRegion = false;
        SpritePixelSize = {0, 0};

        if (!assetManager)
            return;

        if (SpriteHandle != 0 && assetManager->IsSpriteHandle(SpriteHandle))
        {
            SpriteResolved resolved = assetManager->ResolveSprite(SpriteHandle);
            if (resolved.IsValid)
            {
                Texture = resolved.Texture;
                CachedUVs = resolved.UVs;
                Pivot = resolved.Pivot;
                SpritePixelSize = resolved.PixelSize;
                PixelsPerUnit = resolved.PixelsPerUnit;
                UseSpriteRegion = true;
                return;
            }
        }

        if (TextureHandle != 0 && assetManager->IsAssetHandleValid(TextureHandle))
        {
            Ref<Asset> asset = assetManager->GetAsset(TextureHandle);
            if (asset)
            {
                Texture = std::static_pointer_cast<Texture2D>(asset);
                SpritePixelSize = {
                    static_cast<int>(Texture->GetWidth()),
                    static_cast<int>(Texture->GetHeight())
                };

                const TextureImportData* importData = assetManager->GetTextureImportData(TextureHandle);
                if (importData && importData->PixelsPerUnit > 0)
                    PixelsPerUnit = importData->PixelsPerUnit;
            }
        }
    }

    glm::mat4 SpriteRendererComponent::GetVisualTransform(const TransformComponent& transform) const
    {
        const glm::mat4 entityTransform = transform.GetTransform();
        if (SpritePixelSize.x > 0 && SpritePixelSize.y > 0 && PixelsPerUnit > 0)
        {
            return SpriteSheetUtility::BuildSpriteRenderTransform(
                entityTransform, SpritePixelSize, PixelsPerUnit, Pivot);
        }

        return entityTransform;
    }

} // namespace Himii
