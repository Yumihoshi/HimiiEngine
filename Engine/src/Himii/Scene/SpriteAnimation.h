#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Himii/Asset/Asset.h"

namespace Himii
{

    class SpriteAnimation : public Asset {
    public:
        SpriteAnimation() = default;
        virtual ~SpriteAnimation() = default;

        virtual AssetType GetType() const override
        {
            return AssetType::SpriteAnimation;
        }

        void AddFrame(AssetHandle frameHandle)
        {
            m_Frames.push_back(frameHandle);
        }

        void AddSpriteFrame(AssetHandle spriteHandle)
        {
            m_Frames.push_back(spriteHandle);
        }

        const std::vector<AssetHandle> &GetFrames() const
        {
            return m_Frames;
        }

        AssetHandle GetFrame(int index) const
        {
            if (index >= 0 && index < (int)m_Frames.size())
                return m_Frames[index];
            return 0;
        }

        size_t GetFrameCount() const
        {
            if (UsesAtlasFrames())
                return m_AtlasFrameCoordinates.size();
            return m_Frames.size();
        }

        bool UsesAtlasFrames() const
        {
            return m_AtlasTextureHandle != 0 && !m_AtlasFrameCoordinates.empty();
        }

        AssetHandle GetAtlasTextureHandle() const { return m_AtlasTextureHandle; }
        uint32_t GetAtlasGridCellSize() const { return m_AtlasGridCellSize; }

        void SetAtlasTexture(AssetHandle textureHandle, uint32_t gridCellSize)
        {
            m_AtlasTextureHandle = textureHandle;
            m_AtlasGridCellSize = gridCellSize > 0 ? gridCellSize : 16;
            m_Frames.clear();
            m_AtlasFrameCoordinates.clear();
        }

        void ClearAtlasFrames()
        {
            m_AtlasFrameCoordinates.clear();
        }

        void AddAtlasFrame(const glm::ivec2 &atlasCoordinates)
        {
            m_AtlasFrameCoordinates.push_back(atlasCoordinates);
        }

        void GenerateAtlasGridFrames(uint32_t columnCount, uint32_t rowCount)
        {
            m_AtlasFrameCoordinates.clear();
            for (uint32_t row = 0; row < rowCount; ++row)
            {
                for (uint32_t column = 0; column < columnCount; ++column)
                    m_AtlasFrameCoordinates.push_back({(int)column, (int)row});
            }
        }

        glm::ivec2 GetAtlasFrameCoordinates(int frameIndex) const
        {
            if (frameIndex >= 0 && frameIndex < (int)m_AtlasFrameCoordinates.size())
                return m_AtlasFrameCoordinates[frameIndex];
            return {0, 0};
        }

        const std::vector<glm::ivec2> &GetAtlasFrameCoordinatesList() const
        {
            return m_AtlasFrameCoordinates;
        }

    private:
        std::vector<AssetHandle> m_Frames;
        AssetHandle m_AtlasTextureHandle = 0;
        uint32_t m_AtlasGridCellSize = 16;
        std::vector<glm::ivec2> m_AtlasFrameCoordinates;
    };
} // namespace Himii
