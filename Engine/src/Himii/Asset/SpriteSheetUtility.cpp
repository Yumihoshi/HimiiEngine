#include "Himii/Asset/SpriteSheetUtility.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace Himii
{

    std::array<glm::vec2, 4> SpriteSheetUtility::PixelRectToUVs(const glm::ivec4& pixelRect,
                                                                uint32_t textureWidth,
                                                                uint32_t textureHeight)
    {
        std::array<glm::vec2, 4> uvs{};
        if (textureWidth == 0 || textureHeight == 0)
        {
            uvs[0] = {0.0f, 0.0f};
            uvs[1] = {1.0f, 0.0f};
            uvs[2] = {1.0f, 1.0f};
            uvs[3] = {0.0f, 1.0f};
            return uvs;
        }

        const float textureWidthFloat = static_cast<float>(textureWidth);
        const float textureHeightFloat = static_cast<float>(textureHeight);

        const float left = static_cast<float>(pixelRect.x) / textureWidthFloat;
        const float right = static_cast<float>(pixelRect.x + pixelRect.z) / textureWidthFloat;

        // PixelRect 使用左上角原点；与默认 DrawQuad 一致，世界空间下方顶点对应纹理 v 较小的一侧
        const float textureCoordinateTop = static_cast<float>(pixelRect.y) / textureHeightFloat;
        const float textureCoordinateBottom =
            static_cast<float>(pixelRect.y + pixelRect.w) / textureHeightFloat;

        uvs[0] = {left, textureCoordinateTop};
        uvs[1] = {right, textureCoordinateTop};
        uvs[2] = {right, textureCoordinateBottom};
        uvs[3] = {left, textureCoordinateBottom};
        return uvs;
    }

    TextureImportData SpriteSheetUtility::CreateDefaultSingleSprite(AssetHandle textureHandle,
                                                                  uint32_t textureWidth,
                                                                  uint32_t textureHeight,
                                                                  const std::string& spriteName)
    {
        TextureImportData importData;
        importData.TextureHandle = textureHandle;
        importData.SpriteMode = TextureSpriteMode::Single;

        SpriteDefinition sprite;
        sprite.Handle = AssetHandle();
        sprite.Name = spriteName;
        sprite.PixelRect = {
            0,
            0,
            static_cast<int>(textureWidth),
            static_cast<int>(textureHeight)
        };
        sprite.Pivot = {0.5f, 0.5f};
        importData.Sprites.push_back(sprite);
        return importData;
    }

    void SpriteSheetUtility::ApplyGridSlice(TextureImportData& importData,
                                            uint32_t textureWidth,
                                            uint32_t textureHeight)
    {
        importData.Sprites.clear();

        const int cellWidth = importData.GridCellSize.x;
        const int cellHeight = importData.GridCellSize.y;
        if (cellWidth <= 0 || cellHeight <= 0 || textureWidth == 0 || textureHeight == 0)
            return;

        const int offsetX = importData.GridOffset.x;
        const int offsetY = importData.GridOffset.y;
        const int paddingX = importData.GridPadding.x;
        const int paddingY = importData.GridPadding.y;

        const int stepX = cellWidth + paddingX;
        const int stepY = cellHeight + paddingY;

        int spriteIndex = 0;
        for (int row = offsetY; row + cellHeight <= static_cast<int>(textureHeight); row += stepY)
        {
            for (int column = offsetX; column + cellWidth <= static_cast<int>(textureWidth); column += stepX)
            {
                SpriteDefinition sprite;
                sprite.Handle = AssetHandle();
                sprite.Name = "sprite_" + std::to_string(spriteIndex);
                sprite.PixelRect = {column, row, cellWidth, cellHeight};
                sprite.Pivot = {0.5f, 0.5f};
                importData.Sprites.push_back(sprite);
                ++spriteIndex;
            }
        }

        importData.SpriteMode = importData.Sprites.size() > 1
            ? TextureSpriteMode::Multiple
            : TextureSpriteMode::Single;
    }

    glm::mat4 SpriteSheetUtility::BuildSpriteRenderTransform(const glm::mat4& entityTransform,
                                                             const glm::ivec2& pixelSize,
                                                             uint32_t pixelsPerUnit,
                                                             const glm::vec2& pivot)
    {
        if (pixelSize.x <= 0 || pixelSize.y <= 0 || pixelsPerUnit == 0)
            return entityTransform;

        const float worldWidth = static_cast<float>(pixelSize.x) / static_cast<float>(pixelsPerUnit);
        const float worldHeight = static_cast<float>(pixelSize.y) / static_cast<float>(pixelsPerUnit);

        const glm::vec3 pivotOffset(
            (0.5f - pivot.x) * worldWidth,
            (0.5f - pivot.y) * worldHeight,
            0.0f);

        const glm::mat4 pivotMatrix = glm::translate(glm::mat4(1.0f), pivotOffset);
        const glm::mat4 localScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(worldWidth, worldHeight, 1.0f));
        return entityTransform * pivotMatrix * localScaleMatrix;
    }

    std::array<glm::vec2, 4> SpriteSheetUtility::AtlasGridCoordsToUVs(const glm::ivec2& atlasCoordinates,
                                                                      uint32_t tilePixelSize,
                                                                      uint32_t textureWidth,
                                                                      uint32_t textureHeight)
    {
        const glm::ivec4 pixelRect(
                atlasCoordinates.x * static_cast<int>(tilePixelSize),
                atlasCoordinates.y * static_cast<int>(tilePixelSize),
                static_cast<int>(tilePixelSize),
                static_cast<int>(tilePixelSize));
        return PixelRectToUVs(pixelRect, textureWidth, textureHeight);
    }

    glm::mat4 SpriteSheetUtility::ComputeSpriteVisualTransform(const glm::mat4& entityTransform,
                                                               const glm::ivec2& pixelSize,
                                                               uint32_t pixelsPerUnit,
                                                               const glm::vec2& pivot,
                                                               bool useSpriteRegion)
    {
        if (useSpriteRegion && pixelSize.x > 0 && pixelSize.y > 0 && pixelsPerUnit > 0)
            return BuildSpriteRenderTransform(entityTransform, pixelSize, pixelsPerUnit, pivot);

        return entityTransform;
    }

} // namespace Himii
