#include "Himii/Asset/SpriteSheetUtility.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace Himii
{

    namespace
    {
        void ComputeGpuUvEdgesFromPixelRect(const glm::ivec4& pixelRect,
                                            uint32_t textureWidth,
                                            uint32_t textureHeight,
                                            float& left,
                                            float& right,
                                            float& gpuCoordinateTop,
                                            float& gpuCoordinateBottom)
        {
            const float textureWidthFloat = static_cast<float>(textureWidth);
            const float textureHeightFloat = static_cast<float>(textureHeight);

            left = static_cast<float>(pixelRect.x) / textureWidthFloat;
            right = static_cast<float>(pixelRect.x + pixelRect.z) / textureWidthFloat;

            // PixelRect：文件左上原点。OpenGL 纹理 v=0 在底边（stbi flip on load 后文件顶在 v=1）。
            gpuCoordinateTop = 1.0f - static_cast<float>(pixelRect.y) / textureHeightFloat;
            gpuCoordinateBottom =
                1.0f - static_cast<float>(pixelRect.y + pixelRect.w) / textureHeightFloat;
        }
    }

    glm::ivec4 SpriteSheetUtility::AtlasGridCoordsToPixelRect(const glm::ivec2& atlasCoordinates,
                                                             uint32_t tilePixelSize)
    {
        const int cellSize = static_cast<int>(tilePixelSize > 0 ? tilePixelSize : 16);
        return {
            atlasCoordinates.x * cellSize,
            atlasCoordinates.y * cellSize,
            cellSize,
            cellSize};
    }

    ImGuiImageUvCorners SpriteSheetUtility::PixelRectToImGuiImageUv(const glm::ivec4& pixelRect,
                                                                  uint32_t textureWidth,
                                                                  uint32_t textureHeight)
    {
        ImGuiImageUvCorners corners{};
        PixelRectToImGuiImageUVCorners(pixelRect, textureWidth, textureHeight,
                                       corners.TopLeft, corners.BottomRight);
        return corners;
    }

    std::array<glm::vec2, 4> SpriteSheetUtility::PixelRectToImGuiQuadUVsForScreenCorners(
            const glm::ivec4& pixelRect,
            uint32_t textureWidth,
            uint32_t textureHeight)
    {
        const ImGuiImageUvCorners corners = PixelRectToImGuiImageUv(pixelRect, textureWidth, textureHeight);
        return {
            glm::vec2(corners.TopLeft.x, corners.BottomRight.y),
            glm::vec2(corners.BottomRight.x, corners.BottomRight.y),
            glm::vec2(corners.BottomRight.x, corners.TopLeft.y),
            glm::vec2(corners.TopLeft.x, corners.TopLeft.y)};
    }

    std::array<glm::vec2, 4> SpriteSheetUtility::PixelRectToWorldQuadUVs(const glm::ivec4& pixelRect,
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

        float left = 0.0f;
        float right = 1.0f;
        float gpuCoordinateTop = 1.0f;
        float gpuCoordinateBottom = 0.0f;
        ComputeGpuUvEdgesFromPixelRect(pixelRect, textureWidth, textureHeight,
                                       left, right, gpuCoordinateTop, gpuCoordinateBottom);

        // DrawQuadUV 顶点顺序：0=BL, 1=BR, 2=TR, 3=TL（世界 Y 向上）
        uvs[0] = {left, gpuCoordinateBottom};
        uvs[1] = {right, gpuCoordinateBottom};
        uvs[2] = {right, gpuCoordinateTop};
        uvs[3] = {left, gpuCoordinateTop};
        return uvs;
    }

    std::array<glm::vec2, 4> SpriteSheetUtility::AtlasGridCoordsToWorldQuadUVs(
            const glm::ivec2& atlasCoordinates,
            uint32_t tilePixelSize,
            uint32_t textureWidth,
            uint32_t textureHeight)
    {
        return PixelRectToWorldQuadUVs(
                AtlasGridCoordsToPixelRect(atlasCoordinates, tilePixelSize),
                textureWidth, textureHeight);
    }

    std::array<glm::vec2, 4> SpriteSheetUtility::PixelRectToUVs(const glm::ivec4& pixelRect,
                                                                uint32_t textureWidth,
                                                                uint32_t textureHeight)
    {
        return PixelRectToWorldQuadUVs(pixelRect, textureWidth, textureHeight);
    }

    void SpriteSheetUtility::PixelRectToImGuiImageUVCorners(const glm::ivec4& pixelRect,
                                                            uint32_t textureWidth,
                                                            uint32_t textureHeight,
                                                            glm::vec2& uvTopLeft,
                                                            glm::vec2& uvBottomRight)
    {
        if (textureWidth == 0 || textureHeight == 0)
        {
            uvTopLeft = {0.0f, 1.0f};
            uvBottomRight = {1.0f, 0.0f};
            return;
        }

        float left = 0.0f;
        float right = 1.0f;
        float gpuCoordinateTop = 1.0f;
        float gpuCoordinateBottom = 0.0f;
        ComputeGpuUvEdgesFromPixelRect(pixelRect, textureWidth, textureHeight,
                                       left, right, gpuCoordinateTop, gpuCoordinateBottom);

        uvTopLeft = {left, gpuCoordinateTop};
        uvBottomRight = {right, gpuCoordinateBottom};
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
