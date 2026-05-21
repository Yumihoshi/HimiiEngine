#pragma once

#include "Himii/Asset/Asset.h"
#include "Himii/Core/Core.h"
#include "Himii/Renderer/Texture.h"

#include <glm/glm.hpp>
#include <cstdint>

#include <functional>
#include <string>

struct ImGuiPayload;
struct ImVec2;

namespace Himii
{

    constexpr float InspectorLabelColumnWidth = 140.0f;

    void DrawPropertyRow(const char* label, const std::function<void()>& drawValueColumn,
                         const char* tooltipText = nullptr);

    /** 对上一项控件在鼠标悬浮时显示提示（通常紧接在 DrawPropertyRow 等之后调用）。 */
    void DrawInspectorTooltipIfHovered(const char* tooltipText);

    void DrawFloatControl(const std::string& label, float& value, float speed = 0.1f,
                          float minimum = 0.0f, float maximum = 0.0f);

    void DrawColorControl(const std::string& label, glm::vec4& value);

    void DrawCheckboxControl(const std::string& label, bool& value);

    void DrawEnumComboControl(const char* label, int& currentIndex, const char* const* labels,
                              int labelCount, const std::function<void(int newIndex)>& onSelectionChanged);

    void DrawUInt32Control(const char* label, uint32_t& value, float speed = 1.0f);

    void DrawIVec2Control(const char* label, glm::ivec2& value, float speed = 1.0f,
                          int minimum = 0, int maximum = 0);

    void DrawIVec4Control(const char* label, glm::ivec4& value, float speed = 1.0f,
                          int minimum = 0, int maximum = 8192,
                          const std::function<void()>& onEdited = nullptr);

    void DrawInputTextControl(const char* label, char* buffer, int bufferSize,
                              const std::function<void()>& onEdited = nullptr);

    void DrawVec2Control(const char* label, glm::vec2& value, float speed = 0.01f,
                         float minimum = 0.0f, float maximum = 1.0f,
                         const std::function<void()>& onEdited = nullptr);

    void DrawReadOnlyTextControl(const char* label, const char* text,
                                 const char* tooltipText = nullptr);

    void DrawInspectorSectionHeader(const char* title, const char* tooltipText = nullptr);

    void DrawActionButtonRow(const char* label, const std::function<void()>& drawButtons);

    /** 与编辑器主工具栏一致：未选中透明，选中时深色底。返回是否在本帧被点击。 */
    bool DrawEditorToggleButton(const char* label, bool isActive, const char* tooltipText = nullptr);
    bool DrawEditorToggleButton(const char* label, bool isActive, const ImVec2& buttonSize,
                                const char* tooltipText = nullptr);

    /** 两列等宽按钮行（用于 Save、批量操作等）。 */
    void DrawHorizontalButtonPair(const char* pairIdentifier,
                                  const std::function<void()>& drawLeftColumn,
                                  const std::function<void()>& drawRightColumn);

    /**
     * 在 Table 单元格内铺满宽度的按钮。勿使用 ImVec2(-1,0)，否则列宽会每帧被错误回传并逐渐变窄。
     */
    bool DrawTableFillButton(const char* label, const char* tooltipText = nullptr);

    bool AssignTextureFromContentBrowserPayload(const ImGuiPayload* payload, Ref<Texture2D>& texture,
                                                AssetHandle& textureHandle);

    bool AssignSpriteFromContentBrowserPayload(const ImGuiPayload* payload, AssetHandle& spriteAssetHandle);

    bool AssignAnimationAssetFromContentBrowserPayload(const ImGuiPayload* payload,
                                                       AssetHandle& animationAssetHandle);

    void DrawObjectReferenceField(const char* label, const char* objectDisplayName, bool hasReference,
                                 const Ref<Texture2D>& previewTexture,
                                 const std::function<void()>& onClear,
                                 const std::function<bool(const ImGuiPayload*)>& onAssignPayload,
                                 const std::function<void()>& onDoubleClick = nullptr);

    /** 编辑器内显示整张纹理（与 SpriteSheetUtility 约定一致，避免上下颠倒）。 */
    void DrawEditorTextureImageFull(uint64_t textureRendererId, const ImVec2& displaySize);

    /** 编辑器内显示 PixelRect 子区域。 */
    void DrawEditorTextureImageSubRect(uint64_t textureRendererId, const ImVec2& displaySize,
                                       const glm::ivec4& pixelRect, uint32_t textureWidth,
                                       uint32_t textureHeight);

} // namespace Himii
