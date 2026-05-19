#include "TextureInspectorPanel.h"

#include "Himii/Asset/AssetManager.h"
#include "Himii/Asset/SpriteSheetUtility.h"
#include "Himii/Project/Project.h"

#include <imgui.h>
#include <algorithm>

namespace Himii
{

    void TextureInspectorPanel::SetTextureHandle(AssetHandle textureHandle)
    {
        m_TextureHandle = textureHandle;
        m_PreviewTexture = nullptr;
        m_SelectedSpriteIndex = 0;
        ReloadImportData();
    }

    void TextureInspectorPanel::ReloadImportData()
    {
        if (m_TextureHandle == 0 || !Project::GetActive())
            return;

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;

        assetManager->EnsureDefaultTextureMeta(m_TextureHandle);
        Ref<Asset> asset = assetManager->GetAsset(m_TextureHandle);
        if (asset)
            m_PreviewTexture = std::static_pointer_cast<Texture2D>(asset);

        const TextureImportData* importData = assetManager->GetTextureImportData(m_TextureHandle);
        if (!importData)
            return;

        m_GridCellSize = importData->GridCellSize;
        m_GridOffset = importData->GridOffset;
        m_GridPadding = importData->GridPadding;
        m_PixelsPerUnit = importData->PixelsPerUnit;
        m_SpriteModeSelection = static_cast<int>(importData->SpriteMode);
    }

    void TextureInspectorPanel::SyncUIToImportData()
    {
        if (m_TextureHandle == 0 || !Project::GetActive())
            return;

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;

        TextureImportData& importData = assetManager->GetOrCreateTextureImportData(m_TextureHandle);
        importData.GridCellSize = m_GridCellSize;
        importData.GridOffset = m_GridOffset;
        importData.GridPadding = m_GridPadding;
        importData.PixelsPerUnit = m_PixelsPerUnit > 0 ? m_PixelsPerUnit : 100;
        importData.SpriteMode = static_cast<TextureSpriteMode>(m_SpriteModeSelection);
    }

    void TextureInspectorPanel::SaveImportData()
    {
        if (m_TextureHandle == 0 || !Project::GetActive())
            return;

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;

        SyncUIToImportData();
        assetManager->SaveTextureImportData(m_TextureHandle);
    }

    void TextureInspectorPanel::OnImGuiRender(bool& isOpen)
    {
        if (!isOpen)
            return;

        ImGui::Begin("Texture Inspector", &isOpen);

        if (m_TextureHandle == 0)
        {
            ImGui::TextWrapped("Select a texture in the Content Browser or assign one on a Sprite Renderer.");
            ImGui::End();
            return;
        }

        ImGui::Text("Texture Handle: %llu", static_cast<uint64_t>(m_TextureHandle));

        if (ImGui::Button("Reload"))
            ReloadImportData();
        ImGui::SameLine();
        if (ImGui::Button("Save Meta"))
            SaveImportData();

        ImGui::Separator();

        DrawImportSettings();
        ImGui::Separator();
        DrawTexturePreview();
        ImGui::Separator();
        DrawSpriteList();

        ImGui::End();
    }

    void TextureInspectorPanel::DrawImportSettings()
    {
        const char* spriteModeLabels[] = {"None", "Single", "Multiple"};
        ImGui::Combo("Sprite Mode", &m_SpriteModeSelection, spriteModeLabels, 3);
        ImGui::DragScalar("Pixels Per Unit", ImGuiDataType_U32, &m_PixelsPerUnit, 1.0f, nullptr, nullptr, "%u");
        if (ImGui::IsItemDeactivatedAfterEdit())
            SaveImportData();

        if (ImGui::Button("Apply Import Settings"))
            SaveImportData();

        ImGui::Separator();
        ImGui::Text("Grid Slice");
        ImGui::DragInt2("Cell Size", &m_GridCellSize.x, 1.0f, 1, 4096);
        ImGui::DragInt2("Offset", &m_GridOffset.x, 1.0f, 0, 4096);
        ImGui::DragInt2("Padding", &m_GridPadding.x, 1.0f, 0, 256);

        if (ImGui::Button("Apply Grid Slice"))
        {
            auto assetManager = Project::GetAssetManager();
            if (assetManager)
            {
                SyncUIToImportData();
                assetManager->ApplyGridSliceToTexture(m_TextureHandle);
                ReloadImportData();
            }
        }
    }

    void TextureInspectorPanel::DrawSpriteList()
    {
        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;

        const std::vector<SpriteDefinition>& sprites = assetManager->GetSpritesForTexture(m_TextureHandle);
        ImGui::Text("Sprites (%zu)", sprites.size());

        if (ImGui::Button("Add Sprite"))
        {
            TextureImportData& importData = assetManager->GetOrCreateTextureImportData(m_TextureHandle);
            SpriteDefinition newSprite;
            newSprite.Handle = AssetHandle();
            newSprite.Name = "sprite_" + std::to_string(importData.Sprites.size());
            if (m_PreviewTexture)
            {
                newSprite.PixelRect = {
                    0, 0,
                    static_cast<int>(m_PreviewTexture->GetWidth()),
                    static_cast<int>(m_PreviewTexture->GetHeight())
                };
            }
            importData.Sprites.push_back(newSprite);
            importData.SpriteMode = TextureSpriteMode::Multiple;
            assetManager->SaveTextureImportData(m_TextureHandle);
            m_SelectedSpriteIndex = static_cast<int>(importData.Sprites.size()) - 1;
            ReloadImportData();
        }

        for (int spriteIndex = 0; spriteIndex < static_cast<int>(sprites.size()); ++spriteIndex)
        {
            ImGui::PushID(spriteIndex);
            const bool isSelected = (spriteIndex == m_SelectedSpriteIndex);
            if (ImGui::Selectable(sprites[spriteIndex].Name.c_str(), isSelected))
                m_SelectedSpriteIndex = spriteIndex;

            if (isSelected)
            {
                TextureImportData& importData = assetManager->GetOrCreateTextureImportData(m_TextureHandle);
                if (spriteIndex < static_cast<int>(importData.Sprites.size()))
                {
                    SpriteDefinition& sprite = importData.Sprites[spriteIndex];
                    char nameBuffer[128];
                    std::snprintf(nameBuffer, sizeof(nameBuffer), "%s", sprite.Name.c_str());
                    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
                        sprite.Name = nameBuffer;

                    ImGui::DragInt4("Pixel Rect", &sprite.PixelRect.x, 1.0f, 0, 8192);
                    ImGui::DragFloat2("Pivot", &sprite.Pivot.x, 0.01f, 0.0f, 1.0f);
                    ImGui::TextDisabled("Handle: %llu", static_cast<uint64_t>(sprite.Handle));

                    if (ImGui::Button("Apply Changes"))
                    {
                        SyncUIToImportData();
                        assetManager->SaveTextureImportData(m_TextureHandle);
                    }

                    if (ImGui::Button("Delete Sprite"))
                    {
                        importData.Sprites.erase(importData.Sprites.begin() + spriteIndex);
                        assetManager->SaveTextureImportData(m_TextureHandle);
                        m_SelectedSpriteIndex = std::max(0, spriteIndex - 1);
                        ReloadImportData();
                        ImGui::PopID();
                        break;
                    }
                }
            }
            ImGui::PopID();
        }
    }

    void TextureInspectorPanel::DrawTexturePreview()
    {
        if (!m_PreviewTexture)
            return;

        ImGui::Text("Preview (drag on image to define sprite rect, top-left origin)");

        const float previewWidth = ImGui::GetContentRegionAvail().x;
        const float aspectRatio = static_cast<float>(m_PreviewTexture->GetWidth()) /
                                  static_cast<float>(m_PreviewTexture->GetHeight());
        const float previewHeight = previewWidth / aspectRatio;

        const ImVec2 imagePosition = ImGui::GetCursorScreenPos();
        ImGui::Image((ImTextureID)(intptr_t)m_PreviewTexture->GetRendererID(),
                     ImVec2(previewWidth, previewHeight), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;

        const std::vector<SpriteDefinition>& sprites = assetManager->GetSpritesForTexture(m_TextureHandle);
        const float scaleX = previewWidth / static_cast<float>(m_PreviewTexture->GetWidth());
        const float scaleY = previewHeight / static_cast<float>(m_PreviewTexture->GetHeight());

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        for (int spriteIndex = 0; spriteIndex < static_cast<int>(sprites.size()); ++spriteIndex)
        {
            const SpriteDefinition& sprite = sprites[spriteIndex];
            const ImVec2 rectMin(
                imagePosition.x + sprite.PixelRect.x * scaleX,
                imagePosition.y + sprite.PixelRect.y * scaleY);
            const ImVec2 rectMax(
                rectMin.x + sprite.PixelRect.z * scaleX,
                rectMin.y + sprite.PixelRect.w * scaleY);

            const ImU32 color = (spriteIndex == m_SelectedSpriteIndex)
                ? IM_COL32(255, 220, 0, 200)
                : IM_COL32(0, 200, 255, 160);
            drawList->AddRect(rectMin, rectMax, color, 0.0f, 0, 2.0f);
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            const ImVec2 mousePosition = ImGui::GetMousePos();
            m_IsDrawingSpriteRect = true;
            m_DrawStartPixel.x = static_cast<int>((mousePosition.x - imagePosition.x) / scaleX);
            m_DrawStartPixel.y = static_cast<int>((mousePosition.y - imagePosition.y) / scaleY);
            m_DrawEndPixel = m_DrawStartPixel;
        }

        if (m_IsDrawingSpriteRect && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            const ImVec2 mousePosition = ImGui::GetMousePos();
            m_DrawEndPixel.x = static_cast<int>((mousePosition.x - imagePosition.x) / scaleX);
            m_DrawEndPixel.y = static_cast<int>((mousePosition.y - imagePosition.y) / scaleY);

            const ImVec2 dragMin(
                imagePosition.x + std::min(m_DrawStartPixel.x, m_DrawEndPixel.x) * scaleX,
                imagePosition.y + std::min(m_DrawStartPixel.y, m_DrawEndPixel.y) * scaleY);
            const ImVec2 dragMax(
                imagePosition.x + std::max(m_DrawStartPixel.x, m_DrawEndPixel.x) * scaleX,
                imagePosition.y + std::max(m_DrawStartPixel.y, m_DrawEndPixel.y) * scaleY);
            drawList->AddRect(dragMin, dragMax, IM_COL32(255, 80, 80, 220), 0.0f, 0, 2.0f);
        }

        if (m_IsDrawingSpriteRect && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            m_IsDrawingSpriteRect = false;

            const int left = std::clamp(std::min(m_DrawStartPixel.x, m_DrawEndPixel.x), 0,
                                        static_cast<int>(m_PreviewTexture->GetWidth()));
            const int top = std::clamp(std::min(m_DrawStartPixel.y, m_DrawEndPixel.y), 0,
                                       static_cast<int>(m_PreviewTexture->GetHeight()));
            const int right = std::clamp(std::max(m_DrawStartPixel.x, m_DrawEndPixel.x), 0,
                                         static_cast<int>(m_PreviewTexture->GetWidth()));
            const int bottom = std::clamp(std::max(m_DrawStartPixel.y, m_DrawEndPixel.y), 0,
                                          static_cast<int>(m_PreviewTexture->GetHeight()));

            const int width = right - left;
            const int height = bottom - top;
            if (width > 0 && height > 0)
            {
                TextureImportData& importData = assetManager->GetOrCreateTextureImportData(m_TextureHandle);
                if (m_SelectedSpriteIndex >= 0 &&
                    m_SelectedSpriteIndex < static_cast<int>(importData.Sprites.size()))
                {
                    importData.Sprites[m_SelectedSpriteIndex].PixelRect = {left, top, width, height};
                }
                else
                {
                    SpriteDefinition newSprite;
                    newSprite.Handle = AssetHandle();
                    newSprite.Name = "sprite_" + std::to_string(importData.Sprites.size());
                    newSprite.PixelRect = {left, top, width, height};
                    importData.Sprites.push_back(newSprite);
                    m_SelectedSpriteIndex = static_cast<int>(importData.Sprites.size()) - 1;
                }
                importData.SpriteMode = TextureSpriteMode::Multiple;
                assetManager->SaveTextureImportData(m_TextureHandle);
                ReloadImportData();
            }
        }
    }

} // namespace Himii
