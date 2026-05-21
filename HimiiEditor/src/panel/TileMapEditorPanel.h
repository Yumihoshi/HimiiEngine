#pragma once

#include "Himii/Core/Core.h"
#include "Himii/Asset/Asset.h"
#include "Himii/Scene/TileSet.h"
#include "Himii/Scene/TileMapData.h"
#include "Himii/Scene/TileMapCoordinateUtility.h"

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
        bool HasBrushTileSelected() const { return m_BrushTileSelected; }
        bool CanPaintInScene() const;
        bool HasActiveTileMap() const { return m_MapData != nullptr; }

        bool IsMoveEntityModeEnabled() const { return m_MoveEntityModeEnabled; }
        void SetMoveEntityModeEnabled(bool enabled);
        void ToggleMoveEntityMode();

        void ApplyToolAtTile(int tileX, int tileY);
        void SetCurrentTool(Tool tool) { m_CurrentTool = tool; }
        glm::ivec2 GetHoveredTileCoordinates() const { return m_HoveredTileCoordinates; }
        void SetHoveredTileCoordinates(glm::ivec2 coordinates) { m_HoveredTileCoordinates = coordinates; }

        bool SaveActiveTileMapAssets();

        /// 清空笔刷并退出场景绘制会话（关闭 Setup、切换选中、移动实体模式时调用）
        void ClearScenePaintSession();

    private:
        static constexpr float LeftPanelWidth = 320.0f;

        void LoadAssets();
        void SaveTileMap();
        void SaveTileSet();
        void SliceAtlasGrid();

        void DrawLeftPanel();
        void DrawAtlasPreviewPanel();
        void DrawSelectedTileThumbnail();

        void UI_Toolbar();
        void UI_Collision();
        void UI_AtlasSetup();
        void UI_Properties();

        uint16_t FindTileIDAtAtlasCell(int column, int row) const;
        const TileDef* GetSelectedTileDefinition() const;

        void FloodFill(int startX, int startY, uint16_t target, uint16_t replacement);

        AssetHandle m_TileMapHandle = 0;
        Ref<TileMapData> m_MapData;
        Ref<TileSet> m_TileSet;

        Tool m_CurrentTool = Tool::Brush;
        uint16_t m_SelectedTileID = 0;
        bool m_BrushTileSelected = false;

        glm::ivec2 m_HoveredTileCoordinates{TileMapCoordinateUtility::InvalidTileCoordinate,
                                            TileMapCoordinateUtility::InvalidTileCoordinate};

        AssetHandle m_AtlasTextureHandle = 0;
        uint32_t m_AtlasTileSize = 16;
        bool m_MoveEntityModeEnabled = false;
    };

} // namespace Himii
