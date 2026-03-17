#include "ParticleEmitterEditorPanel.h"

#include "imgui.h"
#include <algorithm>
#include <cfloat>
#include <functional>
#include "Himii/Asset/AssetManager.h"
#include "Himii/Asset/AssetSerializer.h"
#include "Himii/Project/Project.h"
#include "Himii/Renderer/Renderer2D.h"
#include "Himii/Renderer/RenderCommand.h"
#include "Himii/Renderer/Texture.h"
#include "Himii/Core/Log.h"
#include "Himii/Particle/ParticleSystem.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Himii
{
    ParticleEmitterEditorPanel::ParticleEmitterEditorPanel()
        : m_Camera(-5.0f, 5.0f, -5.0f, 5.0f)
    {
        FramebufferSpecification fbSpec{ 320, 240 };
        fbSpec.Attachments = { FramebufferFormat::RGBA8, FramebufferFormat::Depth };
        m_Framebuffer = Framebuffer::Create(fbSpec);
    }

    void ParticleEmitterEditorPanel::Open(AssetHandle emitterHandle)
    {
        m_EmitterHandle = emitterHandle;
        m_Asset.reset();
        LoadAsset();
        m_PreviewAccumulator = 0.0f;
    }

    void ParticleEmitterEditorPanel::LoadAsset()
    {
        if (!Project::GetActive() || m_EmitterHandle == 0)
            return;
        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;
        Ref<Asset> ref = assetManager->GetAsset(m_EmitterHandle);
        if (ref)
            m_Asset = std::static_pointer_cast<ParticleEmitterAsset>(ref);
    }

    void ParticleEmitterEditorPanel::UpdatePreview(float deltaTime)
    {
        if (!m_Asset || !m_PreviewPlaying)
        {
            m_PreviewParticleSystem.OnUpdate(deltaTime);
            return;
        }
        m_PreviewAccumulator += deltaTime * m_Asset->EmissionRate;
        int emitCount = static_cast<int>(std::floor(m_PreviewAccumulator));
        if (emitCount > 0)
        {
            m_PreviewAccumulator -= static_cast<float>(emitCount);
            ParticleProps props = m_Asset->TemplateProps;
            props.position = glm::vec3(0.0f, 0.0f, 0.0f);
            for (int i = 0; i < emitCount; ++i)
                m_PreviewParticleSystem.Emit(props);
        }
        m_PreviewParticleSystem.OnUpdate(deltaTime);
    }

    void ParticleEmitterEditorPanel::RenderPreview()
    {
        ImVec2 region = ImGui::GetContentRegionAvail();
        if (region.x < 1.0f || region.y < 1.0f)
            return;

        // 固定 1:1 预览比例，保持正确缩放比：缩小窗口时用黑边而非压缩内容
        float displaySize = std::min(region.x, region.y);
        uint32_t fbSize = static_cast<uint32_t>(displaySize);
        if (fbSize < 1)
            fbSize = 1;
        if (m_Framebuffer->GetSpecification().Width != fbSize || m_Framebuffer->GetSpecification().Height != fbSize)
            m_Framebuffer->Resize(fbSize, fbSize);

        // 正交投影固定 1:1，世界 -5..5 始终对应正方形 viewport
        m_Camera.SetProjection(-5.0f, 5.0f, -5.0f, 5.0f);

        m_Framebuffer->Bind();
        RenderCommand::SetClearColor({ 0.12f, 0.12f, 0.15f, 1.0f });
        RenderCommand::Clear();

        Ref<Texture2D> previewTex;
        if (m_Asset && m_Asset->TemplateProps.textureHandle != 0)
        {
            auto am = Project::GetAssetManager();
            if (am && am->IsAssetHandleValid(static_cast<AssetHandle>(m_Asset->TemplateProps.textureHandle)))
            {
                Ref<Asset> a = am->GetAsset(static_cast<AssetHandle>(m_Asset->TemplateProps.textureHandle));
                previewTex = std::dynamic_pointer_cast<Texture2D>(a);
            }
        }

        Renderer2D::BeginScene(m_Camera);
        m_PreviewParticleSystem.ForEachAlive([&](const ParticleSystem::ParticleView& p)
        {
            float t = 1.0f - p.remainingLife / p.lifetime;
            glm::vec4 color = glm::mix(p.colorBegin, p.colorEnd, t);
            float size = glm::mix(p.sizeBegin, p.sizeEnd, t);
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), p.position)
                * glm::rotate(glm::mat4(1.0f), p.rotation, glm::vec3(0, 0, 1))
                * glm::scale(glm::mat4(1.0f), glm::vec3(size));

            if (p.shape == ParticleShape::Circle)
                Renderer2D::DrawCircle(transform, color, 1.0f, 0.0025f);
            else
            {
                if (previewTex)
                    Renderer2D::DrawQuad(transform, previewTex, 1.0f, color);
                else
                    Renderer2D::DrawQuad(transform, color);
            }
        });
        Renderer2D::EndScene();

        m_Framebuffer->Unbind();

        // 在可用区域内居中显示正方形预览，保持 1:1 不拉伸
        float offsetX = (region.x - displaySize) * 0.5f;
        float offsetY = (region.y - displaySize) * 0.5f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
        uint64_t texId = m_Framebuffer->GetColorAttachmentRendererID(0);
        ImGui::Image(reinterpret_cast<void*>(texId), ImVec2(displaySize, displaySize), { 0, 1 }, { 1, 0 });
    }

    // 属性行：左侧参数名，右侧控件，中间分割（用两列实现）
    static void PropRow(const char* label, const char* tooltip, std::function<void()> widget)
    {
        ImGui::TextUnformatted(label);
        if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("%s", tooltip);
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        widget();
        ImGui::NextColumn();
    }

    void ParticleEmitterEditorPanel::UI_Properties()
    {
        if (!m_Asset)
            return;

        ParticleProps& p = m_Asset->TemplateProps;
        const float labelWidth = 88.0f; // 左侧参数名宽度，避免文字超出

        if (ImGui::CollapsingHeader("发射位置 (模板)", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextDisabled("运行时以实体位置为准");
            ImGui::Columns(2, "##pos", false);
            ImGui::SetColumnWidth(0, labelWidth);
            PropRow("位置", nullptr, [&]() { ImGui::DragFloat3("##pos", &p.position.x, 0.05f); });
            ImGui::Columns(1);
        }

        if (ImGui::CollapsingHeader("初速度", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Columns(2, "##vel", false);
            ImGui::SetColumnWidth(0, labelWidth);
            PropRow("主速度", "平均速度向量，如 (0,1,0) 向上", [&]() { ImGui::DragFloat3("##vel", &p.velocity.x, 0.05f); });
            PropRow("随机扩散", "每轴随机范围，越大越散开", [&]() { ImGui::DragFloat3("##velvar", &p.velocityVariation.x, 0.01f, 0.0f, 10.0f); });
            ImGui::Columns(1);
        }

        if (ImGui::CollapsingHeader("生命周期", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Columns(2, "##life", false);
            ImGui::SetColumnWidth(0, labelWidth);
            PropRow("存活时间", "秒", [&]() { ImGui::DragFloat("##lifetime", &p.lifetime, 0.05f, 0.01f, 10.0f); });
            ImGui::Columns(1);
        }

        if (ImGui::CollapsingHeader("基础图形", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Columns(2, "##shape", false);
            ImGui::SetColumnWidth(0, labelWidth);
            PropRow("形状", "方形=Quad，圆形=Circle", [&]()
            {
                const char* items[] = { "方形 (Quad)", "圆形 (Circle)" };
                int idx = static_cast<int>(p.shape);
                if (ImGui::Combo("##shape", &idx, items, 2))
                    p.shape = static_cast<ParticleShape>(idx);
            });
            PropRow("贴图", "拖入贴图资源，0=仅颜色", [&]()
            {
                auto am = Project::GetAssetManager();
                std::string label = "无";
                if (p.textureHandle != 0 && am && am->IsAssetHandleValid(static_cast<AssetHandle>(p.textureHandle)))
                    label = "贴图 " + std::to_string(static_cast<uint64_t>(p.textureHandle));
                ImGui::Button(label.c_str(), ImVec2(-1, 0));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* path = static_cast<const wchar_t*>(payload->Data);
                        std::filesystem::path assetPath(path);
                        std::string ext = assetPath.extension().string();
                        for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                        if ((ext == ".png" || ext == ".jpg" || ext == ".jpeg") && am)
                        {
                            AssetHandle h = am->ImportAsset(assetPath);
                            if (h != 0) p.textureHandle = static_cast<uint64_t>(h);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                if (p.textureHandle != 0)
                {
                    ImGui::SameLine();
                    if (ImGui::SmallButton("清除"))
                        p.textureHandle = 0;
                }
            });
            ImGui::Columns(1);
        }

        if (ImGui::CollapsingHeader("外观渐变", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Columns(2, "##app", false);
            ImGui::SetColumnWidth(0, labelWidth);
            PropRow("起始颜色", nullptr, [&]() { ImGui::ColorEdit4("##colBeg", &p.colorBegin.x, ImGuiColorEditFlags_NoLabel); });
            PropRow("结束颜色", nullptr, [&]() { ImGui::ColorEdit4("##colEnd", &p.colorEnd.x, ImGuiColorEditFlags_NoLabel); });
            PropRow("起始大小", nullptr, [&]() { ImGui::DragFloat("##szBeg", &p.sizeBegin, 0.01f, 0.0f, 5.0f); });
            PropRow("结束大小", "出生到消失线性插值", [&]() { ImGui::DragFloat("##szEnd", &p.sizeEnd, 0.01f, 0.0f, 5.0f); });
            ImGui::Columns(1);
        }

        if (ImGui::CollapsingHeader("发射器", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Columns(2, "##emit", false);
            ImGui::SetColumnWidth(0, labelWidth);
            PropRow("发射率", "每秒粒子数", [&]() { ImGui::DragFloat("##rate", &m_Asset->EmissionRate, 1.0f, 0.0f, 500.0f, "%.0f"); });
            PropRow("循环", nullptr, [&]() { ImGui::Checkbox("##loop", &m_Asset->Looping); });
            ImGui::Columns(1);
        }
    }

    void ParticleEmitterEditorPanel::SaveAsset()
    {
        if (!m_Asset || m_EmitterHandle == 0 || !Project::GetActive())
            return;
        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return;
        const auto& registry = assetManager->GetAssetRegistry();
        auto it = registry.find(m_EmitterHandle);
        if (it != registry.end())
        {
            std::filesystem::path fullPath = Project::GetAssetFileSystemPath(it->second.FilePath);
            ParticleEmitterAssetSerializer::Serialize(fullPath, m_Asset);
            HIMII_CORE_INFO("ParticleEmitter saved to: {0}", fullPath.string());
        }
    }

    void ParticleEmitterEditorPanel::OnImGuiRender(bool& isOpen)
    {
        if (!isOpen)
            return;

        const float minWinW = 6.0f + MinPreviewWidth + MinPropertiesPanelWidth;
        ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(minWinW, 200.0f), ImVec2(FLT_MAX, FLT_MAX));
        if (!ImGui::Begin("Particle Emitter Editor", &isOpen))
        {
            ImGui::End();
            return;
        }

        if (m_EmitterHandle == 0 || !m_Asset)
        {
            ImGui::TextDisabled("No emitter loaded.");
            ImGui::TextDisabled("Select an entity with ParticleEmitterComponent and click 'Open in Particle Emitter Editor'.");
            ImGui::End();
            return;
        }

        UpdatePreview(ImGui::GetIO().DeltaTime);

        if (ImGui::Button("Save"))
            SaveAsset();
        ImGui::SameLine();
        if (ImGui::Button(m_PreviewPlaying ? "Pause" : "Play"))
            m_PreviewPlaying = !m_PreviewPlaying;
        ImGui::SameLine();
        ImGui::Text("Handle: %llu", (uint64_t)m_EmitterHandle);
        ImGui::Separator();

        const float splitterWidth = 6.0f;
        float totalWidth = ImGui::GetContentRegionAvail().x;
        if (totalWidth < splitterWidth + MinPropertiesPanelWidth + MinPreviewWidth)
            totalWidth = splitterWidth + MinPropertiesPanelWidth + MinPreviewWidth;

        m_PropertiesPanelWidth = std::clamp(m_PropertiesPanelWidth,
            MinPropertiesPanelWidth,
            totalWidth - splitterWidth - MinPreviewWidth);
        float previewWidth = totalWidth - splitterWidth - m_PropertiesPanelWidth;

        ImGui::BeginChild("Preview", ImVec2(previewWidth, -1), true, ImGuiWindowFlags_NoScrollbar);
        RenderPreview();
        ImGui::EndChild();

        ImGui::SameLine(0, 0);

        ImGui::InvisibleButton("##Splitter", ImVec2(splitterWidth, -1));
        bool splitterHovered = ImGui::IsItemHovered();
        if (ImGui::IsItemActive())
        {
            m_PropertiesPanelWidth -= ImGui::GetIO().MouseDelta.x;
            m_PropertiesPanelWidth = std::clamp(m_PropertiesPanelWidth,
                MinPropertiesPanelWidth,
                totalWidth - splitterWidth - MinPreviewWidth);
        }
        if (splitterHovered)
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        // 分割条竖线
        ImVec2 sMin = ImGui::GetItemRectMin();
        ImVec2 sMax = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRectFilled(sMin, sMax,
            splitterHovered ? IM_COL32(100, 100, 100, 255) : IM_COL32(60, 60, 60, 255));

        ImGui::SameLine(0, 0);

        ImGui::BeginChild("Properties", ImVec2(m_PropertiesPanelWidth, -1), true);
        UI_Properties();
        ImGui::EndChild();

        ImGui::End();
    }
}
