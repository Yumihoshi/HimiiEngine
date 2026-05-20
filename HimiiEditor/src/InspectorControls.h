#pragma once

#include "Himii/Asset/Asset.h"
#include "Himii/Core/Core.h"
#include "Himii/Renderer/Texture.h"

#include <glm/glm.hpp>
#include <cstdint>

#include <functional>
#include <string>

struct ImGuiPayload;

namespace Himii
{

    constexpr float InspectorLabelColumnWidth = 140.0f;

    void DrawPropertyRow(const char* label, const std::function<void()>& drawValueColumn);

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

    void DrawReadOnlyTextControl(const char* label, const char* text);

    void DrawInspectorSectionHeader(const char* title);

    void DrawActionButtonRow(const char* label, const std::function<void()>& drawButtons);

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

} // namespace Himii
