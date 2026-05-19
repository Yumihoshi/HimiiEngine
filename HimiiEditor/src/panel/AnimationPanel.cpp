#include "AnimationPanel.h"

#include "Himii/Asset/AssetSerializer.h"
#include "Himii/Asset/Sprite.h"
#include "Himii/Asset/SpriteSheetUtility.h"
#include "Himii/Project/Project.h"
#include "Himii/Core/Log.h"
#include "Himii/Utils/PlatformUtils.h" // 假设你有 FileDialog

#include <filesystem>
#include <algorithm>
#include <cctype>
#include <imgui.h>

namespace Himii
{

    AnimationPanel::AnimationPanel()
    {
        // 启动时创建一个空的动画，避免空指针
        CreateNewAnimation();
    }

    void AnimationPanel::OnImGuiRender(bool &isOpen)
    {
        if (!isOpen)
            return;

        ImGui::Begin("SpriteFrames Editor", &isOpen, ImGuiWindowFlags_MenuBar);

        RenderMenuBar();
        RenderAtlasSetup();
        ImGui::Separator();

        // 使用分栏：左边预览，右边/下边时间轴
        // 这里简化为：上方预览，下方帧列表

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 300.0f); // 预览区域宽度

        RenderPreview();

        ImGui::NextColumn();

        RenderTimeline();

        ImGui::Columns(1);

        ImGui::End();
    }

    void AnimationPanel::RenderMenuBar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Animation"))
                    CreateNewAnimation();

                if (ImGui::MenuItem("Open Animation..."))
                {
                    std::string filepath = FileDialog::OpenFile("Sprite Animation (*.anim)\0*.anim\0");
                    if (!filepath.empty())
                        SetContext(filepath);
                }

                if (ImGui::MenuItem("Save", "Ctrl+S"))
                    SaveAnimation();

                if (ImGui::MenuItem("Save As..."))
                    SaveAnimationAs();

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    void AnimationPanel::RenderPreview()
    {
        ImGui::Text("Preview");
        ImGui::Separator();

        // 播放控制
        if (ImGui::Button(m_IsPlaying ? "Stop" : "Play"))
        {
            m_IsPlaying = !m_IsPlaying;
        }
        ImGui::SameLine();
        ImGui::DragFloat("FPS", &m_FrameRate, 0.5f, 1.0f, 60.0f);

        // 预览逻辑更新
        if (m_CurrentAnimation && m_CurrentAnimation->GetFrameCount() > 0)
        {
            if (m_IsPlaying)
            {
                m_Timer += ImGui::GetIO().DeltaTime;
                if (m_Timer >= (1.0f / m_FrameRate))
                {
                    m_Timer = 0.0f;
                    m_PreviewFrameIndex = (m_PreviewFrameIndex + 1) % m_CurrentAnimation->GetFrameCount();
                }
            }
            else
            {
                // 如果暂停，显示选中的帧，或者第0帧
                if (m_SelectedFrameIndex >= 0 && m_SelectedFrameIndex < m_CurrentAnimation->GetFrameCount())
                    m_PreviewFrameIndex = m_SelectedFrameIndex;
            }

            const float regionWidth = ImGui::GetContentRegionAvail().x;
            DrawFramePreview(m_PreviewFrameIndex, ImVec2(regionWidth, regionWidth));
        }
        else
        {
            ImGui::Text("No frames in animation.");
        }
    }

  void AnimationPanel::RenderTimeline()
    {
        ImGui::Text("Timeline");
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
        {
            if (m_CurrentAnimation)
            {
                // 创建一个新的空动画，相当于清空
                m_CurrentAnimation = std::make_shared<SpriteAnimation>();
                m_SelectedFrameIndex = -1;
            }
        }

        ImGui::Separator();

        // 开启子窗口，这里是滚动区域
        ImGui::BeginChild("FrameList", ImVec2(0, 0), true); // true 显示边框

        if (m_CurrentAnimation)
        {
            const int frameCount = (int)m_CurrentAnimation->GetFrameCount();
            for (int frameIndex = 0; frameIndex < frameCount; frameIndex++)
            {
                ImGui::PushID(frameIndex);

                const ImVec2 thumbnailSize = {64, 64};
                if (frameIndex == m_SelectedFrameIndex)
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 0, 1));

                AnimationFramePreview preview = ResolveFramePreview(frameIndex);
                bool clicked = false;
                if (preview.Valid && preview.Texture)
                {
                    if (ImGui::ImageButton("frame", (uint64_t)preview.Texture->GetRendererID(), thumbnailSize,
                                           preview.UV0, preview.UV1))
                        clicked = true;
                }
                else if (ImGui::Button("?", thumbnailSize))
                    clicked = true;

                if (frameIndex == m_SelectedFrameIndex)
                    ImGui::PopStyleColor();

                if (clicked)
                {
                    m_SelectedFrameIndex = frameIndex;
                    m_PreviewFrameIndex = frameIndex;
                    m_IsPlaying = false;
                }

                ImGui::SameLine();
                ImGui::PopID();
            }
        }

        // 2. 换行，防止最后的 Dummy 跟在最后一个图片后面
        ImGui::NewLine();

        // 3. 关键修复：创建一个填充剩余空间的 Dummy 控件
        // 这样即使列表是空的，或者图片很少，下方大片空白区域也能响应拖拽
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        if (contentSize.y < 50.0f)
            contentSize.y = 50.0f; // 保证至少有 50px 的高度

        ImGui::Dummy(contentSize); // 不可见的控件，用来接收拖拽

        // 4. 在 Dummy 上附加拖拽目标逻辑
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t *path = (const wchar_t *)payload->Data;
                std::filesystem::path assetPath = path;

                HIMII_CORE_INFO("Dropped file: {0}", assetPath.string()); // [DEBUG] 打印日志

                std::string ext = assetPath.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
                    ImportTextureAsAtlas(assetPath);
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::EndChild();
    }

    void AnimationPanel::CreateNewAnimation()
    {
        m_CurrentAnimation = std::make_shared<SpriteAnimation>();
        m_CurrentFilePath = "";
        m_PreviewFrameIndex = 0;
        m_SelectedFrameIndex = -1;
        m_IsPlaying = false;
    }

    void AnimationPanel::SetContext(const std::filesystem::path &path)
    {
        Ref<SpriteAnimation> anim = SpriteAnimationSerializer::Deserialize(path);
        if (anim)
        {
            m_CurrentAnimation = anim;
            m_CurrentFilePath = path;
            m_PreviewFrameIndex = 0;
            m_SelectedFrameIndex = -1;
        }
    }

    void AnimationPanel::SaveAnimationAs()
    {
        std::string filepath = FileDialog::SaveFile("Sprite Animation (*.anim)\0*.anim\0");
        if (!filepath.empty())
        {
            m_CurrentFilePath = filepath;
            SaveAnimation();
        }
    }

    void AnimationPanel::SaveAnimation()
    {
        if (m_CurrentFilePath.empty())
        {
            SaveAnimationAs();
        }
        else
        {
            if (m_CurrentAnimation)
            {
                SpriteAnimationSerializer::Serialize(m_CurrentFilePath, m_CurrentAnimation);

                // 如果这个文件之前没有被 AssetManager 注册，这里应该触发一次导入
                // 但通常 Serializer 是直接写文件，AssetManager 下次启动或 Refresh 时会发现它
                // 也可以手动调用: Project::GetAssetManager()->ImportAsset(m_CurrentFilePath);
            }
        }
    }

    void AnimationPanel::RenderAtlasSetup()
    {
        ImGui::Text("Atlas (Godot SpriteFrames)");
        ImGui::DragInt("Grid Cell Size", (int *)&m_AtlasGridCellSize, 1.0f, 1, 512);

        if (m_CurrentAnimation && m_CurrentAnimation->UsesAtlasFrames())
        {
            ImGui::TextDisabled("Atlas texture handle: %llu",
                                (uint64_t)m_CurrentAnimation->GetAtlasTextureHandle());
            ImGui::TextDisabled("Frames: %zu", m_CurrentAnimation->GetFrameCount());
        }

        if (ImGui::Button("Slice Grid (replace frames)"))
        {
            if (!m_CurrentAnimation || m_CurrentAnimation->GetAtlasTextureHandle() == 0)
                return;

            auto assetManager = Project::GetAssetManager();
            if (!assetManager)
                return;

            Ref<Texture2D> texture = std::static_pointer_cast<Texture2D>(
                    assetManager->GetAsset(m_CurrentAnimation->GetAtlasTextureHandle()));
            if (!texture)
                return;

            const uint32_t cellSize = m_AtlasGridCellSize > 0 ? m_AtlasGridCellSize : 16;
            const uint32_t columnCount = std::max(1u, texture->GetWidth() / cellSize);
            const uint32_t rowCount = std::max(1u, texture->GetHeight() / cellSize);
            m_CurrentAnimation->GenerateAtlasGridFrames(columnCount, rowCount);
            m_PreviewFrameIndex = 0;
            m_SelectedFrameIndex = 0;
        }
    }

    void AnimationPanel::ImportTextureAsAtlas(const std::filesystem::path &assetPath)
    {
        auto assetManager = Project::GetAssetManager();
        if (!assetManager || !m_CurrentAnimation)
            return;

        AssetHandle textureHandle = assetManager->ImportAsset(assetPath);
        if (textureHandle == 0)
            return;

        m_CurrentAnimation->SetAtlasTexture(textureHandle, m_AtlasGridCellSize);

        Ref<Texture2D> texture =
                std::static_pointer_cast<Texture2D>(assetManager->GetAsset(textureHandle));
        if (!texture)
            return;

        const uint32_t cellSize = m_AtlasGridCellSize > 0 ? m_AtlasGridCellSize : 16;
        const uint32_t columnCount = std::max(1u, texture->GetWidth() / cellSize);
        const uint32_t rowCount = std::max(1u, texture->GetHeight() / cellSize);
        m_CurrentAnimation->GenerateAtlasGridFrames(columnCount, rowCount);
        m_PreviewFrameIndex = 0;
        m_SelectedFrameIndex = 0;
    }

    AnimationPanel::AnimationFramePreview AnimationPanel::ResolveFramePreview(int frameIndex) const
    {
        AnimationFramePreview preview;
        if (!m_CurrentAnimation)
            return preview;

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return preview;

        if (m_CurrentAnimation->UsesAtlasFrames())
        {
            const glm::ivec2 atlasCoordinates = m_CurrentAnimation->GetAtlasFrameCoordinates(frameIndex);
            const uint32_t cellSize = m_CurrentAnimation->GetAtlasGridCellSize();
            Ref<Texture2D> texture = std::static_pointer_cast<Texture2D>(
                    assetManager->GetAsset(m_CurrentAnimation->GetAtlasTextureHandle()));
            if (!texture)
                return preview;

            const glm::ivec4 pixelRect(atlasCoordinates.x * (int)cellSize, atlasCoordinates.y * (int)cellSize,
                                       (int)cellSize, (int)cellSize);
            const auto uvs = SpriteSheetUtility::PixelRectToUVs(pixelRect, texture->GetWidth(), texture->GetHeight());

            preview.Texture = texture;
            preview.UV0 = ImVec2(uvs[0].x, uvs[0].y);
            preview.UV1 = ImVec2(uvs[2].x, uvs[2].y);
            preview.Valid = true;
            return preview;
        }

        AssetHandle frameHandle = m_CurrentAnimation->GetFrame(frameIndex);
        if (assetManager->IsSpriteHandle(frameHandle))
        {
            SpriteResolved resolved = assetManager->ResolveSprite(frameHandle);
            if (!resolved.IsValid || !resolved.Texture)
                return preview;

            preview.Texture = resolved.Texture;
            preview.UV0 = ImVec2(resolved.UVs[0].x, resolved.UVs[0].y);
            preview.UV1 = ImVec2(resolved.UVs[2].x, resolved.UVs[2].y);
            preview.Valid = true;
            return preview;
        }

        preview.Texture = GetTextureFromHandle(frameHandle);
        preview.Valid = preview.Texture != nullptr;
        return preview;
    }

    void AnimationPanel::DrawFramePreview(int frameIndex, const ImVec2 &displaySize)
    {
        AnimationFramePreview preview = ResolveFramePreview(frameIndex);
        if (preview.Valid && preview.Texture)
        {
            ImGui::Image((void *)(uint64_t)preview.Texture->GetRendererID(), displaySize, preview.UV0,
                         preview.UV1);
        }
        else
        {
            ImGui::Text("Invalid frame %d", frameIndex);
        }
    }

    Ref<Texture2D> AnimationPanel::GetTextureFromHandle(AssetHandle handle) const
    {
        if (handle == 0)
            return nullptr;

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return nullptr;

        if (assetManager->IsSpriteHandle(handle))
        {
            SpriteResolved resolved = assetManager->ResolveSprite(handle);
            return resolved.Texture;
        }

        if (assetManager->IsAssetHandleValid(handle))
            return std::static_pointer_cast<Texture2D>(assetManager->GetAsset(handle));

        return nullptr;
    }
} // namespace Himii
