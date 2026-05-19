#pragma once

#include "Himii/Core/Core.h"
#include "Himii/Asset/Asset.h"
#include "Himii/Renderer/Texture.h"

namespace Himii
{

    class TextureInspectorPanel
    {
    public:
        void OnImGuiRender(bool& isOpen);

        void SetTextureHandle(AssetHandle textureHandle);
        AssetHandle GetTextureHandle() const { return m_TextureHandle; }

    private:
        void DrawImportSettings();
        void DrawSpriteList();
        void DrawTexturePreview();
        void ReloadImportData();
        void SyncUIToImportData();
        void SaveImportData();

        AssetHandle m_TextureHandle = 0;
        Ref<Texture2D> m_PreviewTexture;

        int m_SelectedSpriteIndex = 0;
        bool m_IsDrawingSpriteRect = false;
        glm::ivec2 m_DrawStartPixel{0, 0};
        glm::ivec2 m_DrawEndPixel{0, 0};

        glm::ivec2 m_GridCellSize{32, 32};
        glm::ivec2 m_GridOffset{0, 0};
        glm::ivec2 m_GridPadding{0, 0};
        int m_SpriteModeSelection = 1;
        uint32_t m_PixelsPerUnit = 100;
    };

} // namespace Himii
