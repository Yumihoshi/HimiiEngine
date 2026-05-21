#include "TileMapEditorPanel.h"
#include "InspectorControls.h"
#include "TilemapEditorUtility.h"

#include "imgui.h"
#include "Himii/Asset/AssetManager.h"
#include "Himii/Asset/AssetSerializer.h"
#include "Himii/Asset/SpriteSheetUtility.h"
#include "Himii/Scene/TileMapCoordinateUtility.h"
#include "Himii/Project/Project.h"
#include "Himii/Core/Log.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <queue>
#include <unordered_set>

namespace Himii
{

    void TileMapEditorPanel::Open(AssetHandle tileMapHandle)
    {
        m_TileMapHandle = tileMapHandle;
        m_MapData = nullptr;
        m_TileSet = nullptr;
        m_AtlasTextureHandle = 0;
        m_SelectedTileID = 0;
        m_BrushTileSelected = false;
        LoadAssets();
    }

    void TileMapEditorPanel::ClearScenePaintSession()
    {
        m_SelectedTileID = 0;
        m_BrushTileSelected = false;
        m_CurrentTool = Tool::Brush;
        m_MoveEntityModeEnabled = false;
        m_HoveredTileCoordinates = {TileMapCoordinateUtility::InvalidTileCoordinate,
                                    TileMapCoordinateUtility::InvalidTileCoordinate};
    }

    void TileMapEditorPanel::SetMoveEntityModeEnabled(bool enabled)
    {
        if (enabled)
        {
            m_SelectedTileID = 0;
            m_BrushTileSelected = false;
            m_CurrentTool = Tool::Brush;
        }
        m_MoveEntityModeEnabled = enabled;
    }

    void TileMapEditorPanel::ToggleMoveEntityMode()
    {
        SetMoveEntityModeEnabled(!m_MoveEntityModeEnabled);
    }

    bool TileMapEditorPanel::CanPaintInScene() const
    {
        if (!m_MapData || m_MoveEntityModeEnabled)
            return false;

        if (m_CurrentTool == Tool::Eraser)
            return true;

        return m_BrushTileSelected && m_SelectedTileID != 0;
    }

    void TileMapEditorPanel::LoadAssets()
    {
        if (!Project::GetActive() || m_TileMapHandle == 0)
            return;

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;

        auto mapAsset = assetManager->GetAsset(m_TileMapHandle);
        if (mapAsset)
            m_MapData = std::static_pointer_cast<TileMapData>(mapAsset);

        if (m_MapData && m_MapData->GetTileSetHandle() != 0)
        {
            auto tileSetAsset = assetManager->GetAsset(m_MapData->GetTileSetHandle());
            if (tileSetAsset)
            {
                m_TileSet = std::static_pointer_cast<TileSet>(tileSetAsset);
                if (!m_TileSet->GetAtlasSources().empty())
                {
                    m_AtlasTextureHandle = m_TileSet->GetAtlasSources()[0].TextureHandle;
                    m_AtlasTileSize = m_TileSet->GetAtlasSources()[0].TileSize;
                }
            }
        }
    }

    const TileDef* TileMapEditorPanel::GetSelectedTileDefinition() const
    {
        if (!m_TileSet)
            return nullptr;
        return m_TileSet->GetTileDef(m_SelectedTileID);
    }

    uint16_t TileMapEditorPanel::FindTileIDAtAtlasCell(int column, int row) const
    {
        if (!m_TileSet)
            return 0;

        for (const auto& [tileIdentifier, tileDefinition] : m_TileSet->GetTileDefs())
        {
            if (tileDefinition.SourceType == TileSourceType::Atlas
                && tileDefinition.AtlasSourceIndex == 0
                && tileDefinition.AtlasCoords.x == column
                && tileDefinition.AtlasCoords.y == row)
            {
                return tileIdentifier;
            }
        }

        return 0;
    }

    void TileMapEditorPanel::OnImGuiRender(bool &isOpen)
    {
        if (!isOpen)
            return;

        ImGui::SetNextWindowSize(ImVec2(960.0f, 600.0f), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("TileMap Setup", &isOpen))
        {
            ImGui::End();
            return;
        }

        if (m_TileMapHandle == 0 || !m_MapData)
        {
            ImGui::TextDisabled("No TileMap loaded.");
            ImGui::TextWrapped(
                "Select an entity with TilemapComponent, or use Open TileMap Setup from the Inspector.");
            ImGui::End();
            return;
        }

        if (ImGui::BeginTable("##TileMapSetupSplit", 2,
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV
                                  | ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableSetupColumn("Tools", ImGuiTableColumnFlags_WidthFixed, LeftPanelWidth);
            ImGui::TableSetupColumn("AtlasPreview", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            {
                const ImVec2 leftPanelSize = ImGui::GetContentRegionAvail();
                if (ImGui::BeginChild("##TileMapSetupLeft", leftPanelSize, false,
                                      ImGuiWindowFlags_AlwaysVerticalScrollbar))
                {
                    DrawLeftPanel();
                }
                ImGui::EndChild();
            }

            ImGui::TableNextColumn();
            {
                const ImVec2 rightPanelSize = ImGui::GetContentRegionAvail();
                if (ImGui::BeginChild("##TileMapSetupRight", rightPanelSize, true,
                                      ImGuiWindowFlags_NoScrollbar))
                {
                    DrawAtlasPreviewPanel();
                }
                ImGui::EndChild();
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }

    void TileMapEditorPanel::DrawLeftPanel()
    {
        if (auto assetManager = Project::GetAssetManager())
        {
            const auto& registry = assetManager->GetAssetRegistry();
            const auto iterator = registry.find(m_TileMapHandle);
            if (iterator != registry.end())
                ImGui::Text("TileMap: %s", iterator->second.FilePath.filename().string().c_str());
        }

        ImGui::Text("%ux%u  |  Cell: %.2f", m_MapData->GetWidth(), m_MapData->GetHeight(),
                    m_MapData->GetCellSize());
        ImGui::TextDisabled("在右侧图集点选图块后再于 Scene 绘制。Ctrl+S 保存。");
        if (!m_BrushTileSelected && m_CurrentTool != Tool::Eraser)
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "未选择笔刷图块。");

        const bool moveEntityHighlight = m_MoveEntityModeEnabled;
        if (moveEntityHighlight)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.45f, 0.2f, 1.0f));
        if (ImGui::Button(m_MoveEntityModeEnabled ? "Move Entity: ON [H]" : "Move Entity: OFF [H]",
                          ImVec2(-1.0f, 0.0f)))
        {
            ToggleMoveEntityMode();
        }
        if (moveEntityHighlight)
            ImGui::PopStyleColor();
        ImGui::TextDisabled("Hold Alt in Scene to move entity temporarily.");

        ImGui::Separator();
        UI_Toolbar();
        ImGui::Separator();
        UI_Collision();
        ImGui::Separator();
        UI_AtlasSetup();
        ImGui::Separator();
        DrawSelectedTileThumbnail();
        ImGui::Separator();
        UI_Properties();
    }

    void TileMapEditorPanel::UI_Toolbar()
    {
        const bool isBrush = (m_CurrentTool == Tool::Brush);
        const bool isEraser = (m_CurrentTool == Tool::Eraser);
        const bool isFill = (m_CurrentTool == Tool::Fill);

        if (isBrush)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.6f, 0.9f, 1.0f));
        if (ImGui::Button("Brush [B]"))
            m_CurrentTool = Tool::Brush;
        if (isBrush)
            ImGui::PopStyleColor();

        ImGui::SameLine();

        if (isEraser)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("Eraser [E]"))
            m_CurrentTool = Tool::Eraser;
        if (isEraser)
            ImGui::PopStyleColor();

        ImGui::SameLine();

        if (isFill)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
        if (ImGui::Button("Fill [G]"))
            m_CurrentTool = Tool::Fill;
        if (isFill)
            ImGui::PopStyleColor();

        ImGui::Text("Tile ID: %d", (int)m_SelectedTileID);

        if (TileMapCoordinateUtility::IsValidTileCoordinate(m_HoveredTileCoordinates))
            ImGui::TextDisabled("Scene hover: (%d, %d)", m_HoveredTileCoordinates.x,
                                m_HoveredTileCoordinates.y);

        const TileDef* selectedTileDefinition = GetSelectedTileDefinition();
        if (selectedTileDefinition && selectedTileDefinition->SourceType == TileSourceType::Atlas)
        {
            ImGui::TextDisabled("Atlas cell: (%d, %d)", selectedTileDefinition->AtlasCoords.x,
                                selectedTileDefinition->AtlasCoords.y);
        }

    }

    void TileMapEditorPanel::UI_Collision()
    {
        DrawInspectorSectionHeader("Collision");

        if (!m_TileSet)
        {
            ImGui::TextDisabled("Assign a TileSet to configure collision.");
            return;
        }

        uint32_t collidableDefinitionCount = 0;
        const uint32_t totalDefinitionCount = static_cast<uint32_t>(m_TileSet->GetTileDefs().size());
        for (const auto& [tileIdentifier, tileDefinition] : m_TileSet->GetTileDefs())
        {
            if (tileDefinition.Collidable)
                collidableDefinitionCount++;
        }

        ImGui::TextDisabled(
                "Collidable is per tile TYPE (not per painted cell). Save TileSet before Play.");
        ImGui::Text("Tile definitions: %u / %u Collidable",
                    collidableDefinitionCount,
                    totalDefinitionCount);

        if (ImGui::Button("Set All Collidable", ImVec2(-1.0f, 0.0f)))
        {
            for (auto& [tileIdentifier, tileDefinition] : m_TileSet->GetTileDefs())
                tileDefinition.Collidable = true;
            SaveTileSet();
        }

        if (ImGui::Button("Clear All Collidable", ImVec2(-1.0f, 0.0f)))
        {
            for (auto& [tileIdentifier, tileDefinition] : m_TileSet->GetTileDefs())
                tileDefinition.Collidable = false;
            SaveTileSet();
        }

        if (m_SelectedTileID != 0)
        {
            auto& tileDefinitions = m_TileSet->GetTileDefs();
            const auto iterator = tileDefinitions.find(m_SelectedTileID);
            if (iterator != tileDefinitions.end())
            {
                ImGui::Separator();
                DrawCheckboxControl("Selected Tile Collidable", iterator->second.Collidable);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    SaveTileSet();
            }
        }
    }

    void TileMapEditorPanel::UI_AtlasSetup()
    {
        DrawInspectorSectionHeader("Atlas Grid");

        if (!m_TileSet)
        {
            ImGui::TextDisabled("No TileSet assigned to this TileMap.");
            return;
        }

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;

        if (ImGui::DragInt("Tile Size (px)", (int *)&m_AtlasTileSize, 1.0f, 1, 512))
        {
            if (!m_TileSet->GetAtlasSources().empty())
                m_TileSet->GetAtlasSources()[0].TileSize = m_AtlasTileSize;
        }

        DrawActionButtonRow("Atlas", [&]()
        {
            if (ImGui::Button("Slice Grid", ImVec2(-1.0f, 0.0f)))
                SliceAtlasGrid();
        });

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t *path = (const wchar_t *)payload->Data;
                std::filesystem::path assetPath = path;
                std::string extension = assetPath.extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(),
                               [](unsigned char character)
                               {
                                   return (char)std::tolower(character);
                               });
                if (extension == ".png" || extension == ".jpg" || extension == ".jpeg")
                {
                    AssetHandle textureHandle = assetManager->ImportAsset(assetPath);
                    if (textureHandle != 0)
                    {
                        m_AtlasTextureHandle = textureHandle;
                        TileAtlasSource atlasSource;
                        atlasSource.TextureHandle = textureHandle;
                        atlasSource.TileSize = m_AtlasTileSize;
                        m_TileSet->GetAtlasSources().clear();
                        m_TileSet->AddAtlasSource(atlasSource);
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::TextDisabled("Drag texture onto Atlas Grid section or preview.");
    }

    void TileMapEditorPanel::DrawSelectedTileThumbnail()
    {
        const TileDef* selectedTileDefinition = GetSelectedTileDefinition();
        if (!selectedTileDefinition || !m_TileSet)
        {
            ImGui::TextDisabled("Click a cell in the atlas preview to select a brush tile.");
            return;
        }

        auto assetManager = Project::GetAssetManager();
        if (!assetManager || m_AtlasTextureHandle == 0)
            return;

        Ref<Texture2D> atlasTexture =
                std::static_pointer_cast<Texture2D>(assetManager->GetAsset(m_AtlasTextureHandle));
        if (!atlasTexture || selectedTileDefinition->SourceType != TileSourceType::Atlas)
            return;

        const uint32_t tilePixelSize = m_AtlasTileSize > 0 ? m_AtlasTileSize : 16;
        const glm::ivec4 pixelRect = SpriteSheetUtility::AtlasGridCoordsToPixelRect(
                selectedTileDefinition->AtlasCoords, tilePixelSize);

        const float thumbnailSize = 72.0f;
        DrawPropertyRow("Selected Tile", [&]()
        {
            DrawEditorTextureImageSubRect(
                atlasTexture->GetRendererID(), ImVec2(thumbnailSize, thumbnailSize),
                pixelRect, atlasTexture->GetWidth(), atlasTexture->GetHeight());
        });
    }

    void TileMapEditorPanel::DrawAtlasPreviewPanel()
    {
        if (!m_TileSet)
        {
            ImGui::TextDisabled("Assign a TileSet to this TileMap.");
            return;
        }

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;

        Ref<Texture2D> atlasTexture;
        if (m_AtlasTextureHandle != 0)
            atlasTexture = std::static_pointer_cast<Texture2D>(assetManager->GetAsset(m_AtlasTextureHandle));

        if (!atlasTexture)
        {
            ImGui::TextDisabled("Drag a texture into the left Atlas Grid area.");
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t *path = (const wchar_t *)payload->Data;
                    std::filesystem::path assetPath = path;
                    std::string extension = assetPath.extension().string();
                    std::transform(extension.begin(), extension.end(), extension.begin(),
                                   [](unsigned char character)
                                   {
                                       return (char)std::tolower(character);
                                   });
                    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg")
                    {
                        AssetHandle textureHandle = assetManager->ImportAsset(assetPath);
                        if (textureHandle != 0)
                        {
                            m_AtlasTextureHandle = textureHandle;
                            TileAtlasSource atlasSource;
                            atlasSource.TextureHandle = textureHandle;
                            atlasSource.TileSize = m_AtlasTileSize;
                            m_TileSet->GetAtlasSources().clear();
                            m_TileSet->AddAtlasSource(atlasSource);
                            atlasTexture = std::static_pointer_cast<Texture2D>(
                                    assetManager->GetAsset(m_AtlasTextureHandle));
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }
            return;
        }

        const float previewWidth = ImGui::GetContentRegionAvail().x;
        const float previewHeight = ImGui::GetContentRegionAvail().y;
        const float textureAspect = static_cast<float>(atlasTexture->GetWidth())
            / static_cast<float>(atlasTexture->GetHeight());
        const float panelAspect = previewWidth / previewHeight;

        float displayWidth = previewWidth;
        float displayHeight = previewHeight;
        if (textureAspect > panelAspect)
            displayHeight = displayWidth / textureAspect;
        else
            displayWidth = displayHeight * textureAspect;

        const float offsetX = (previewWidth - displayWidth) * 0.5f;
        const float offsetY = (previewHeight - displayHeight) * 0.5f;
        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + offsetX, ImGui::GetCursorPosY() + offsetY));

        const ImVec2 imagePosition = ImGui::GetCursorScreenPos();
        DrawEditorTextureImageFull(atlasTexture->GetRendererID(), ImVec2(displayWidth, displayHeight));

        const uint32_t tilePixelSize = m_AtlasTileSize > 0 ? m_AtlasTileSize : 16;
        const uint32_t columnCount =
                std::max(1u, atlasTexture->GetWidth() / tilePixelSize);
        const uint32_t rowCount =
                std::max(1u, atlasTexture->GetHeight() / tilePixelSize);

        const float scaleX = displayWidth / static_cast<float>(atlasTexture->GetWidth());
        const float scaleY = displayHeight / static_cast<float>(atlasTexture->GetHeight());
        const float cellScreenWidth = static_cast<float>(tilePixelSize) * scaleX;
        const float cellScreenHeight = static_cast<float>(tilePixelSize) * scaleY;

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        for (uint32_t row = 0; row < rowCount; ++row)
        {
            for (uint32_t column = 0; column < columnCount; ++column)
            {
                const ImVec2 rectMin(
                        imagePosition.x + static_cast<float>(column) * cellScreenWidth,
                        imagePosition.y + static_cast<float>(row) * cellScreenHeight);
                const ImVec2 rectMax(rectMin.x + cellScreenWidth, rectMin.y + cellScreenHeight);

                const uint16_t cellTileIdentifier = FindTileIDAtAtlasCell(static_cast<int>(column),
                                                                          static_cast<int>(row));
                const bool isSelectedCell = cellTileIdentifier != 0
                    && cellTileIdentifier == m_SelectedTileID;

                if (cellTileIdentifier != 0)
                    drawList->AddRectFilled(rectMin, rectMax, IM_COL32(0, 200, 255, 20));

                if (isSelectedCell)
                {
                    drawList->AddRectFilled(rectMin, rectMax, IM_COL32(255, 220, 0, 45));
                    drawList->AddRect(rectMin, rectMax, IM_COL32(255, 220, 0, 255), 0.0f, 0, 2.0f);
                }
                else
                    drawList->AddRect(rectMin, rectMax, IM_COL32(100, 100, 100, 120), 0.0f, 0, 1.0f);
            }
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            const ImVec2 mousePosition = ImGui::GetMousePos();
            const float localX = mousePosition.x - imagePosition.x;
            const float localY = mousePosition.y - imagePosition.y;

            if (localX >= 0.0f && localY >= 0.0f && localX < displayWidth && localY < displayHeight)
            {
                const int column = static_cast<int>(localX / cellScreenWidth);
                const int row = static_cast<int>(localY / cellScreenHeight);

                if (column >= 0 && row >= 0
                    && column < static_cast<int>(columnCount)
                    && row < static_cast<int>(rowCount))
                {
                    const uint16_t pickedTileIdentifier = FindTileIDAtAtlasCell(column, row);
                    if (pickedTileIdentifier != 0)
                    {
                        m_SelectedTileID = pickedTileIdentifier;
                        m_BrushTileSelected = true;
                        if (m_CurrentTool == Tool::Eraser)
                            m_CurrentTool = Tool::Brush;
                    }
                }
            }
        }

        if (m_TileSet->GetTileDefs().empty())
        {
            ImGui::SetCursorScreenPos(imagePosition);
            ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "Click Slice Grid to generate tiles.");
        }
    }

    void TileMapEditorPanel::SliceAtlasGrid()
    {
        if (!m_TileSet || m_AtlasTextureHandle == 0)
            return;

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;

        Ref<Texture2D> texture =
                std::static_pointer_cast<Texture2D>(assetManager->GetAsset(m_AtlasTextureHandle));
        if (!texture)
            return;

        if (m_TileSet->GetAtlasSources().empty())
        {
            TileAtlasSource atlasSource;
            atlasSource.TextureHandle = m_AtlasTextureHandle;
            atlasSource.TileSize = m_AtlasTileSize;
            m_TileSet->AddAtlasSource(atlasSource);
        }
        else
        {
            m_TileSet->GetAtlasSources()[0].TextureHandle = m_AtlasTextureHandle;
            m_TileSet->GetAtlasSources()[0].TileSize = m_AtlasTileSize;
        }

        const std::unordered_map<uint16_t, TileDef> previousDefinitions = m_TileSet->GetTileDefs();

        const uint32_t tileSize = m_AtlasTileSize > 0 ? m_AtlasTileSize : 16;
        const uint32_t columnCount = std::max(1u, texture->GetWidth() / tileSize);
        const uint32_t rowCount = std::max(1u, texture->GetHeight() / tileSize);
        m_TileSet->GenerateGridTileDefs(0, columnCount, rowCount, &previousDefinitions);

        m_SelectedTileID = 0;
        m_BrushTileSelected = false;

        HIMII_CORE_INFO("TileSet sliced: {0}x{1} tiles", columnCount, rowCount);
    }

    void TileMapEditorPanel::UI_Properties()
    {
        DrawInspectorSectionHeader("Map Properties");
        if (!m_MapData)
            return;

        float cellSize = m_MapData->GetCellSize();

        if (m_MapData->HasBounds())
        {
            int32_t minTileX = 0;
            int32_t minTileY = 0;
            int32_t maxTileX = 0;
            int32_t maxTileY = 0;
            m_MapData->GetBounds(minTileX, minTileY, maxTileX, maxTileY);
            ImGui::TextDisabled("Bounds: (%d,%d) .. (%d,%d)", minTileX, minTileY, maxTileX, maxTileY);
            ImGui::TextDisabled("Extent: %u x %u tiles", m_MapData->GetWidth(), m_MapData->GetHeight());
        }
        else
        {
            ImGui::TextDisabled("Bounds: empty (paint in Scene to expand)");
        }

        if (ImGui::DragFloat("Cell Size", &cellSize, 0.05f, 0.1f, 10.0f))
            m_MapData->SetCellSize(cellSize);

        DrawActionButtonRow("Save", [&]()
        {
            if (ImGui::Button("Save TileMap", ImVec2(-1.0f, 0.0f)))
                SaveTileMap();
            ImGui::SameLine();
            if (ImGui::Button("Save TileSet", ImVec2(-1.0f, 0.0f)))
                SaveTileSet();
        });
        ImGui::TextDisabled("Ctrl+S saves project, TileMap and TileSet.");
    }

    void TileMapEditorPanel::ApplyToolAtTile(int tileX, int tileY)
    {
        if (!m_MapData)
            return;

        switch (m_CurrentTool)
        {
            case Tool::Brush:
                m_MapData->SetTile(tileX, tileY, m_SelectedTileID);
                break;
            case Tool::Eraser:
                m_MapData->SetTile(tileX, tileY, 0);
                break;
            case Tool::Fill:
            {
                uint16_t target = m_MapData->GetTile(tileX, tileY);
                if (target != m_SelectedTileID)
                    FloodFill(tileX, tileY, target, m_SelectedTileID);
                break;
            }
        }
    }

    void TileMapEditorPanel::FloodFill(int startX, int startY, uint16_t target, uint16_t replacement)
    {
        if (!m_MapData || target == replacement)
            return;

        auto packCoordinates = [](int32_t x, int32_t y) -> int64_t
        {
            return (static_cast<int64_t>(x) << 32) | static_cast<uint32_t>(y);
        };

        std::queue<glm::ivec2> queue;
        std::unordered_set<int64_t> visited;
        queue.push({startX, startY});
        visited.insert(packCoordinates(startX, startY));

        int filledCount = 0;
        constexpr int maxFill = 65536;

        while (!queue.empty() && filledCount < maxFill)
        {
            const glm::ivec2 position = queue.front();
            queue.pop();
            const int32_t x = position.x;
            const int32_t y = position.y;

            if (m_MapData->GetTile(x, y) != target)
                continue;

            m_MapData->SetTile(x, y, replacement);
            filledCount++;

            const glm::ivec2 neighbors[4] = {
                {x + 1, y}, {x - 1, y}, {x, y + 1}, {x, y - 1}};

            for (const glm::ivec2& neighbor : neighbors)
            {
                const int64_t packed = packCoordinates(neighbor.x, neighbor.y);
                if (visited.insert(packed).second)
                    queue.push(neighbor);
            }
        }
    }

    void TileMapEditorPanel::SaveTileMap()
    {
        if (!m_MapData || m_TileMapHandle == 0 || !Project::GetActive())
            return;

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;

        auto &registry = assetManager->GetAssetRegistry();
        auto iterator = registry.find(m_TileMapHandle);
        if (iterator != registry.end())
        {
            auto fullPath = Project::GetAssetFileSystemPath(iterator->second.FilePath);
            TileMapDataSerializer::Serialize(fullPath, m_MapData);
            HIMII_CORE_INFO("TileMap saved to: {0}", fullPath.string());
        }
    }

    void TileMapEditorPanel::SaveTileSet()
    {
        if (!m_TileSet || !m_MapData || !Project::GetActive())
            return;

        const AssetHandle tileSetHandle = m_MapData->GetTileSetHandle();
        if (tileSetHandle == 0)
            return;

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;

        auto &registry = assetManager->GetAssetRegistry();
        auto iterator = registry.find(tileSetHandle);
        if (iterator != registry.end())
        {
            auto fullPath = Project::GetAssetFileSystemPath(iterator->second.FilePath);
            TileSetSerializer::Serialize(fullPath, m_TileSet);
            HIMII_CORE_INFO("TileSet saved: {0}", fullPath.string());
        }
    }

    bool TileMapEditorPanel::SaveActiveTileMapAssets()
    {
        if (!HasActiveTileMap())
            return false;

        SaveTileMap();
        SaveTileSet();
        return true;
    }

} // namespace Himii
