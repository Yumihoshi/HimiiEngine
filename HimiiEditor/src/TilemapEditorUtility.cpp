#include "TilemapEditorUtility.h"

#include <array>
#include <cmath>

namespace Himii
{

    glm::vec2 TilemapEditorUtility::GetMapLocalOrigin(const TileMapData& mapData)
    {
        const float cellSize = mapData.GetCellSize();
        const float offsetX = -(float)mapData.GetWidth() * cellSize * 0.5f;
        const float offsetY = -(float)mapData.GetHeight() * cellSize * 0.5f;
        return {offsetX, offsetY};
    }

    glm::ivec2 TilemapEditorUtility::LocalPositionToTileCoordinates(const glm::vec2& localPosition,
                                                                    const TileMapData& mapData)
    {
        const glm::vec2 origin = GetMapLocalOrigin(mapData);
        const float cellSize = mapData.GetCellSize();
        if (cellSize <= 0.0f)
            return {-1, -1};

        const int tileX = (int)std::floor((localPosition.x - origin.x) / cellSize);
        const int tileY = (int)std::floor((localPosition.y - origin.y) / cellSize);

        if (tileX < 0 || tileY < 0
            || tileX >= (int)mapData.GetWidth() || tileY >= (int)mapData.GetHeight())
            return {-1, -1};

        return {tileX, tileY};
    }

    glm::vec3 TilemapEditorUtility::ViewportMouseToWorld(const glm::vec2& viewportMousePosition,
                                                         const glm::vec2& viewportSize,
                                                         const glm::mat4& viewProjectionMatrix)
    {
        if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f)
            return {0.0f, 0.0f, 0.0f};

        const float normalizedX = (viewportMousePosition.x / viewportSize.x) * 2.0f - 1.0f;
        const float normalizedY = (viewportMousePosition.y / viewportSize.y) * 2.0f - 1.0f;
        glm::vec4 clipSpace = {normalizedX, normalizedY, 0.0f, 1.0f};
        glm::vec4 worldSpace = glm::inverse(viewProjectionMatrix) * clipSpace;
        if (worldSpace.w != 0.0f)
            worldSpace /= worldSpace.w;
        return ViewportMouseToWorldOnPlane(viewportMousePosition, viewportSize, viewProjectionMatrix, 0.0f);
    }

    glm::vec3 TilemapEditorUtility::ViewportMouseToWorldOnPlane(const glm::vec2& viewportMousePosition,
                                                                const glm::vec2& viewportSize,
                                                                const glm::mat4& viewProjectionMatrix,
                                                                float planeZ)
    {
        if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f)
            return {0.0f, 0.0f, planeZ};

        const float normalizedX = (viewportMousePosition.x / viewportSize.x) * 2.0f - 1.0f;
        const float normalizedY = (viewportMousePosition.y / viewportSize.y) * 2.0f - 1.0f;

        const glm::mat4 inverseViewProjection = glm::inverse(viewProjectionMatrix);

        glm::vec4 nearPoint = inverseViewProjection * glm::vec4(normalizedX, normalizedY, -1.0f, 1.0f);
        glm::vec4 farPoint = inverseViewProjection * glm::vec4(normalizedX, normalizedY, 1.0f, 1.0f);
        nearPoint /= nearPoint.w;
        farPoint /= farPoint.w;

        const glm::vec3 rayOrigin = glm::vec3(nearPoint);
        const glm::vec3 rayDirection = glm::normalize(glm::vec3(farPoint - nearPoint));

        if (std::abs(rayDirection.z) < 1.0e-6f)
            return {rayOrigin.x, rayOrigin.y, planeZ};

        const float intersectionDistance = (planeZ - rayOrigin.z) / rayDirection.z;
        return rayOrigin + rayDirection * intersectionDistance;
    }

    glm::vec2 TilemapEditorUtility::WorldToViewportScreen(const glm::vec3& worldPosition,
                                                          const glm::vec2& viewportSize,
                                                          const glm::vec2& viewportOrigin,
                                                          const glm::mat4& viewProjectionMatrix)
    {
        glm::vec4 clipSpace = viewProjectionMatrix * glm::vec4(worldPosition, 1.0f);
        if (clipSpace.w != 0.0f)
            clipSpace /= clipSpace.w;

        const float screenX = (clipSpace.x * 0.5f + 0.5f) * viewportSize.x + viewportOrigin.x;
        const float screenY = viewportOrigin.y + (1.0f - (clipSpace.y * 0.5f + 0.5f)) * viewportSize.y;
        return {screenX, screenY};
    }

    void TilemapEditorUtility::AtlasCoordsToImGuiImageUVs(const glm::ivec2& atlasCoordinates,
                                                            uint32_t tilePixelSize,
                                                            uint32_t textureWidth,
                                                            uint32_t textureHeight,
                                                            glm::vec2& outUvMin,
                                                            glm::vec2& outUvMax)
    {
        if (textureWidth == 0 || textureHeight == 0 || tilePixelSize == 0)
        {
            outUvMin = {0.0f, 1.0f};
            outUvMax = {1.0f, 0.0f};
            return;
        }

        const float unitU = static_cast<float>(tilePixelSize) / static_cast<float>(textureWidth);
        const float unitV = static_cast<float>(tilePixelSize) / static_cast<float>(textureHeight);
        const float coordinateU0 = atlasCoordinates.x * unitU;
        const float coordinateV0 = atlasCoordinates.y * unitV;

        outUvMin = {coordinateU0, coordinateV0 + unitV};
        outUvMax = {coordinateU0 + unitU, coordinateV0};
    }

    std::array<glm::vec2, 4> TilemapEditorUtility::AtlasCoordsToImGuiQuadUVs(
            const glm::ivec2& atlasCoordinates,
            uint32_t tilePixelSize,
            uint32_t textureWidth,
            uint32_t textureHeight)
    {
        glm::vec2 uvMin;
        glm::vec2 uvMax;
        AtlasCoordsToImGuiImageUVs(atlasCoordinates, tilePixelSize, textureWidth, textureHeight, uvMin, uvMax);

        return {
            glm::vec2(uvMin.x, uvMin.y),
            glm::vec2(uvMax.x, uvMin.y),
            glm::vec2(uvMax.x, uvMax.y),
            glm::vec2(uvMin.x, uvMax.y)};
    }

} // namespace Himii
