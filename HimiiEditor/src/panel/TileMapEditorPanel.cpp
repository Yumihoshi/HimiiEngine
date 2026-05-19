#include "TileMapEditorPanel.h"

#include "imgui.h"
#include "Himii/Asset/AssetManager.h"
#include "Himii/Asset/AssetSerializer.h"
#include "Himii/Project/Project.h"
#include "Himii/Core/Log.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <queue>

namespace Himii
{

    void TileMapEditorPanel::Open(AssetHandle tileMapHandle)
    {
        m_TileMapHandle = tileMapHandle;
        m_MapData = nullptr;
        m_TileSet = nullptr;
        m_AtlasTextureHandle = 0;
        LoadAssets();
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

    void TileMapEditorPanel::OnImGuiRender(bool &isOpen)
    {
        if (!isOpen)
            return;

        ImGui::SetNextWindowSize(ImVec2(420, 560), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("TileMap Setup", &isOpen))
        {
            ImGui::End();
            return;
        }

        if (m_TileMapHandle == 0 || !m_MapData)
        {
            ImGui::TextDisabled("No TileMap loaded.");
            ImGui::TextWrapped("Select an entity with TilemapComponent and open TileMap Setup.");
            ImGui::End();
            return;
        }

        ImGui::Text("TileMap: %llu  |  %ux%u  |  Cell: %.2f",
                    (uint64_t)m_TileMapHandle,
                    m_MapData->GetWidth(), m_MapData->GetHeight(),
                    m_MapData->GetCellSize());
        ImGui::TextDisabled("Paint Mode [P]: draw in Scene. Off: move Tilemap with Gizmo.");
        const bool paintModeHighlight = m_PaintModeEnabled;
        if (paintModeHighlight)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        if (ImGui::Button(m_PaintModeEnabled ? "Paint Mode: ON" : "Paint Mode: OFF", ImVec2(-1, 0)))
            TogglePaintMode();
        if (paintModeHighlight)
            ImGui::PopStyleColor();
        ImGui::Separator();

        UI_Toolbar();
        ImGui::Separator();
        UI_AtlasSetup();
        ImGui::Separator();
        UI_TilePalette();
        ImGui::Separator();
        UI_Properties();

        ImGui::End();
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

        ImGui::SameLine();
        ImGui::Text("|  Tile: %d", (int)m_SelectedTileID);

        if (m_HoveredTileCoordinates.x >= 0)
        {
            ImGui::SameLine();
            ImGui::TextDisabled("Hover: (%d, %d)", m_HoveredTileCoordinates.x, m_HoveredTileCoordinates.y);
        }
    }

    void TileMapEditorPanel::UI_AtlasSetup()
    {
        ImGui::Text("Atlas Grid");
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

        if (ImGui::Button("Slice Grid"))
            SliceAtlasGrid();

        ImGui::SameLine();
        if (ImGui::Button("Save TileSet"))
        {
            AssetHandle tileSetHandle = m_MapData->GetTileSetHandle();
            if (tileSetHandle != 0)
            {
                auto &registry = assetManager->GetAssetRegistry();
                auto iterator = registry.find(tileSetHandle);
                if (iterator != registry.end())
                {
                    auto fullPath = Project::GetAssetFileSystemPath(iterator->second.FilePath);
                    TileSetSerializer::Serialize(fullPath, m_TileSet);
                    HIMII_CORE_INFO("TileSet saved: {0}", fullPath.string());
                }
            }
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t *path = (const wchar_t *)payload->Data;
                std::filesystem::path assetPath = path;
                std::string extension = assetPath.extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(),
                               [](unsigned char character) { return (char)std::tolower(character); });
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
        ImGui::TextDisabled("Drag texture here to set atlas.");
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

        const uint32_t tileSize = m_AtlasTileSize > 0 ? m_AtlasTileSize : 16;
        const uint32_t columnCount = std::max(1u, texture->GetWidth() / tileSize);
        const uint32_t rowCount = std::max(1u, texture->GetHeight() / tileSize);
        m_TileSet->GenerateGridTileDefs(0, columnCount, rowCount);

        if (!m_TileSet->GetTileDefs().empty())
            m_SelectedTileID = m_TileSet->GetTileDefs().begin()->second.ID;

        HIMII_CORE_INFO("TileSet sliced: {0}x{1} tiles", columnCount, rowCount);
    }

    void TileMapEditorPanel::UI_TilePalette()
    {
        ImGui::Text("Tile Palette");
        if (!m_TileSet)
        {
            ImGui::TextDisabled("Slice atlas grid first.");
            return;
        }

        const auto &tileDefinitions = m_TileSet->GetTileDefs();
        if (tileDefinitions.empty())
        {
            ImGui::TextDisabled("No tiles — click Slice Grid.");
            return;
        }

        auto assetManager = Project::GetAssetManager();
        Ref<Texture2D> atlasTexture;
        float tilePixelSize = (float)m_AtlasTileSize;
        if (assetManager && m_AtlasTextureHandle != 0)
            atlasTexture = std::static_pointer_cast<Texture2D>(assetManager->GetAsset(m_AtlasTextureHandle));

        const float buttonSize = 40.0f;
        const float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / (buttonSize + 4.0f));
        if (columnCount < 1)
            columnCount = 1;

        int column = 0;
        for (const auto &[tileIdentifier, tileDefinition] : tileDefinitions)
        {
            if (column > 0)
                ImGui::SameLine(0, 4.0f);

            ImGui::PushID((int)tileIdentifier);

            const bool isSelected = (m_SelectedTileID == tileIdentifier);

            bool clicked = false;
            if (atlasTexture && tileDefinition.SourceType == TileSourceType::Atlas && tilePixelSize > 0.0f)
            {
                const float textureWidth = (float)atlasTexture->GetWidth();
                const float textureHeight = (float)atlasTexture->GetHeight();
                const float unitU = tilePixelSize / textureWidth;
                const float unitV = tilePixelSize / textureHeight;
                const float coordinateU0 = tileDefinition.AtlasCoords.x * unitU;
                const float coordinateV0 = tileDefinition.AtlasCoords.y * unitV;
                const float coordinateU1 = coordinateU0 + unitU;
                const float coordinateV1 = coordinateV0 + unitV;

                if (ImGui::ImageButton("tile", (void *)(uint64_t)atlasTexture->GetRendererID(),
                                       ImVec2(buttonSize, buttonSize),
                                       ImVec2(coordinateU0, coordinateV1), ImVec2(coordinateU1, coordinateV0)))
                    clicked = true;
            }
            else
            {
                char label[16];
                snprintf(label, sizeof(label), "%d", (int)tileIdentifier);
                if (ImGui::Button(label, ImVec2(buttonSize, buttonSize)))
                    clicked = true;
            }

            if (isSelected)
            {
                const ImVec2 rectMin = ImGui::GetItemRectMin();
                const ImVec2 rectMax = ImGui::GetItemRectMax();
                ImGui::GetWindowDrawList()->AddRect(rectMin, rectMax, IM_COL32(80, 180, 255, 255), 0.0f, 0, 2.0f);
            }

            if (clicked)
            {
                m_SelectedTileID = tileIdentifier;
                if (m_CurrentTool == Tool::Eraser)
                    m_CurrentTool = Tool::Brush;
            }

            ImGui::PopID();
            column++;
            if (column >= columnCount)
                column = 0;
        }
    }

    void TileMapEditorPanel::UI_Properties()
    {
        ImGui::Text("Map Properties");
        if (!m_MapData)
            return;

        int halfWidth = (int)m_MapData->GetHalfWidth();
        int halfHeight = (int)m_MapData->GetHalfHeight();
        float cellSize = m_MapData->GetCellSize();

        if (ImGui::DragInt("Half W", &halfWidth, 1.0f, 0, 512))
            m_MapData->Resize((uint32_t)(halfWidth > 0 ? halfWidth : 0), m_MapData->GetHalfHeight());
        if (ImGui::DragInt("Half H", &halfHeight, 1.0f, 0, 512))
            m_MapData->Resize(m_MapData->GetHalfWidth(), (uint32_t)(halfHeight > 0 ? halfHeight : 0));
        if (ImGui::DragFloat("Cell Size", &cellSize, 0.05f, 0.1f, 10.0f))
            m_MapData->SetCellSize(cellSize);

        if (ImGui::Button("Save TileMap", ImVec2(-1, 0)))
            SaveTileMap();
    }

    void TileMapEditorPanel::ApplyToolAtTile(int tileX, int tileY)
    {
        if (!m_MapData)
            return;

        switch (m_CurrentTool)
        {
            case Tool::Brush:
                m_MapData->SetTile((uint32_t)tileX, (uint32_t)tileY, m_SelectedTileID);
                break;
            case Tool::Eraser:
                m_MapData->SetTile((uint32_t)tileX, (uint32_t)tileY, 0);
                break;
            case Tool::Fill:
            {
                uint16_t target = m_MapData->GetTile((uint32_t)tileX, (uint32_t)tileY);
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

        const int width = (int)m_MapData->GetWidth();
        const int height = (int)m_MapData->GetHeight();

        std::queue<glm::ivec2> queue;
        queue.push({startX, startY});

        int filledCount = 0;
        const int maxFill = width * height;

        while (!queue.empty() && filledCount < maxFill)
        {
            glm::ivec2 position = queue.front();
            queue.pop();
            const int x = position.x;
            const int y = position.y;

            if (x < 0 || x >= width || y < 0 || y >= height)
                continue;
            if (m_MapData->GetTile((uint32_t)x, (uint32_t)y) != target)
                continue;

            m_MapData->SetTile((uint32_t)x, (uint32_t)y, replacement);
            filledCount++;

            queue.push({x + 1, y});
            queue.push({x - 1, y});
            queue.push({x, y + 1});
            queue.push({x, y - 1});
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

} // namespace Himii
