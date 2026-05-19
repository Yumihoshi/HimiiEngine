#pragma once

#include "Himii/Core/Core.h"
#include "Himii/Renderer/Texture.h"
#include "Himii/Scene/SpriteAnimation.h"

#include <imgui.h>
#include <filesystem>

namespace Himii
{

    class AnimationPanel {
    public:
        AnimationPanel();

        void OnImGuiRender(bool &isOpen);

        // 用于从外部打开文件（例如双击 ContentBrowser 中的文件）
        void SetContext(const std::filesystem::path &path);

    private:
        void RenderMenuBar();
        void RenderPreview();
        void RenderTimeline();

        void CreateNewAnimation();
        void SaveAnimationAs();
        void SaveAnimation();

        Ref<Texture2D> GetTextureFromHandle(AssetHandle handle) const;
        void RenderAtlasSetup();
        void DrawFramePreview(int frameIndex, const ImVec2 &displaySize);
        void ImportTextureAsAtlas(const std::filesystem::path &assetPath);

    private:
        struct AnimationFramePreview
        {
            Ref<Texture2D> Texture;
            ImVec2 UV0{0.0f, 1.0f};
            ImVec2 UV1{1.0f, 0.0f};
            bool Valid = false;
        };

        AnimationFramePreview ResolveFramePreview(int frameIndex) const;

    private:
        Ref<SpriteAnimation> m_CurrentAnimation;
        std::filesystem::path m_CurrentFilePath; // 如果是新建的，可能为空

        // 预览播放状态
        bool m_IsPlaying = false;
        float m_Timer = 0.0f;
        int m_PreviewFrameIndex = 0;
        float m_FrameRate = 10.0f; // 编辑器预览用的帧率，实际运行时使用组件上的设定

        // UI 状态
        int m_SelectedFrameIndex = -1;
        uint32_t m_AtlasGridCellSize = 16;
    };

} // namespace Himii
