#pragma once

#include "Himii/Asset/Sprite.h"

#include <glm/glm.hpp>

namespace Himii
{

    class SpriteSheetUtility
    {
    public:
        static std::array<glm::vec2, 4> PixelRectToUVs(const glm::ivec4& pixelRect,
                                                       uint32_t textureWidth,
                                                       uint32_t textureHeight);

        /** ImGui::Image 与全图预览一致：左上角为原点，需相对 OpenGL 纹理坐标翻转 V。 */
        static void PixelRectToImGuiImageUVCorners(const glm::ivec4& pixelRect,
                                                   uint32_t textureWidth,
                                                   uint32_t textureHeight,
                                                   glm::vec2& uvTopLeft,
                                                   glm::vec2& uvBottomRight);

        static TextureImportData CreateDefaultSingleSprite(AssetHandle textureHandle,
                                                           uint32_t textureWidth,
                                                           uint32_t textureHeight,
                                                           const std::string& spriteName = "default");

        static void ApplyGridSlice(TextureImportData& importData,
                                   uint32_t textureWidth,
                                   uint32_t textureHeight);

        static glm::mat4 BuildSpriteRenderTransform(const glm::mat4& entityTransform,
                                                    const glm::ivec2& pixelSize,
                                                    uint32_t pixelsPerUnit,
                                                    const glm::vec2& pivot);

        /** 与 DrawSprite 一致的可视变换（不修改 TransformComponent::Scale）。 */
        static glm::mat4 ComputeSpriteVisualTransform(const glm::mat4& entityTransform,
                                                      const glm::ivec2& pixelSize,
                                                      uint32_t pixelsPerUnit,
                                                      const glm::vec2& pivot,
                                                      bool useSpriteRegion);

        static std::array<glm::vec2, 4> AtlasGridCoordsToUVs(const glm::ivec2& atlasCoordinates,
                                                             uint32_t tilePixelSize,
                                                             uint32_t textureWidth,
                                                             uint32_t textureHeight);

        /** 将 PixelRectToUVs 结果重排为 DrawQuadUV 顶点顺序：BL、BR、TR、TL（Y 轴向上世界空间）。 */
        static std::array<glm::vec2, 4> ReorderUVsForYUpWorldQuad(
            const std::array<glm::vec2, 4>& textureCoordinates);
    };

} // namespace Himii
