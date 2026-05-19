#pragma once

#include "Himii/Core/Core.h"
#include "Himii/Asset/Asset.h"
#include "Himii/Scene/TileSet.h"
#include "Himii/Scene/TileMapData.h"

#include <glm/glm.hpp>
#include <queue>

namespace Himii
{

    class TileMapEditorPanel {
    public:
        TileMapEditorPanel() = default;

        void OnImGuiRender(bool &isOpen);
        void Open(AssetHandle tileMapHandle);

        enum class Tool { Brush, Eraser, Fill };

        AssetHandle GetTileMapHandle() const { return m_TileMapHandle; }
        Ref<TileMapData> GetMapData() const { return m_MapData; }
        Ref<TileSet> GetTileSet() const { return m_TileSet; }
        Tool GetCurrentTool() const { return m_CurrentTool; }
        uint16_t GetSelectedTileID() const { return m_SelectedTileID; }
        bool HasActiveTileMap() const { return m_MapData != nullptr; }

        bool IsPaintModeEnabled() const { return m_PaintModeEnabled; }
        void SetPaintModeEnabled(bool enabled) { m_PaintModeEnabled = enabled; }
        void TogglePaintMode() { m_PaintModeEnabled = !m_PaintModeEnabled; }

        void ApplyToolAtTile(int tileX, int tileY);
        void SetCurrentTool(Tool tool) { m_CurrentTool = tool; }
        glm::ivec2 GetHoveredTileCoordinates() const { return m_HoveredTileCoordinates; }
        void SetHoveredTileCoordinates(glm::ivec2 coordinates) { m_HoveredTileCoordinates = coordinates; }

    private:
        void LoadAssets();
        void SaveTileMap();
        void SliceAtlasGrid();

        void UI_Toolbar();
        void UI_TilePalette();
        void UI_AtlasSetup();
        void UI_Properties();

        void FloodFill(int startX, int startY, uint16_t target, uint16_t replacement);

        AssetHandle m_TileMapHandle = 0;
        Ref<TileMapData> m_MapData;
        Ref<TileSet> m_TileSet;

        Tool m_CurrentTool = Tool::Brush;
        uint16_t m_SelectedTileID = 1;

        glm::ivec2 m_HoveredTileCoordinates{-1, -1};

        AssetHandle m_AtlasTextureHandle = 0;
        uint32_t m_AtlasTileSize = 16;
        bool m_PaintModeEnabled = false;
    };

} // namespace Himii
