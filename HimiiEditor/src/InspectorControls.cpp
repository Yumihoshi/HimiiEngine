#include "Hepch.h"
#include "InspectorControls.h"

#include "Himii/Asset/AssetManager.h"
#include "Himii/Asset/SpriteSheetUtility.h"
#include "Himii/Project/Project.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <cstring>
#include <filesystem>

namespace Himii
{

    void DrawInspectorTooltipIfHovered(const char* tooltipText)
    {
        if (!tooltipText || tooltipText[0] == '\0')
            return;

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("%s", tooltipText);
    }

    void DrawPropertyRow(const char* label, const std::function<void()>& drawValueColumn,
                         const char* tooltipText)
    {
        ImGui::PushID(label);
        if (ImGui::BeginTable("##PropertyRow", 2,
                              ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, InspectorLabelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(label);
            ImGui::TableNextColumn();
            drawValueColumn();
            ImGui::EndTable();
        }
        DrawInspectorTooltipIfHovered(tooltipText);
        ImGui::PopID();
    }

    void DrawFloatControl(const std::string& label, float& value, float speed, float minimum, float maximum)
    {
        DrawPropertyRow(label.c_str(), [&]()
        {
            ImGui::PushItemWidth(-1.0f);
            ImGui::DragFloat("##Value", &value, speed, minimum, maximum);
            ImGui::PopItemWidth();
        });
    }

    void DrawIntControl(const char* label, int& value, float speed, int minimum, int maximum)
    {
        DrawPropertyRow(label, [&]()
        {
            ImGui::PushItemWidth(-1.0f);
            if (minimum != 0 || maximum != 0)
                ImGui::DragInt("##Value", &value, speed, minimum, maximum);
            else
                ImGui::DragInt("##Value", &value, speed);
            ImGui::PopItemWidth();
        });
    }

    void DrawColorControl(const std::string& label, glm::vec4& value)
    {
        DrawPropertyRow(label.c_str(), [&]()
        {
            ImGui::PushItemWidth(-1.0f);
            ImGui::ColorEdit4("##Value", glm::value_ptr(value));
            ImGui::PopItemWidth();
        });
    }

    void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue,
                         const std::function<void()>& onEditBegin,
                         const std::function<void()>& onEditEnd)
    {
        ImGuiIO& inputOutput = ImGui::GetIO();
        ImFont* boldFont = inputOutput.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());
        if (ImGui::BeginTable("##Vec3Control", 2,
                              ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, InspectorLabelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(label.c_str());
            ImGui::TableNextColumn();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.0f, 0.0f});

            const float lineHeight = GImGui->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            const ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};
            const float widthEach = (ImGui::GetContentRegionAvail().x - 3.0f * buttonSize.x) / 3.0f;

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.23f, 0.12f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
            ImGui::PushFont(boldFont);
            if (ImGui::Button("X", buttonSize))
                values.x = resetValue;
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::SetNextItemWidth(widthEach);
            ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
            if (ImGui::IsItemActivated() && onEditBegin)
                onEditBegin();
            if (ImGui::IsItemDeactivatedAfterEdit() && onEditEnd)
                onEditEnd();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.12f, 0.7f, 0.2f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
            ImGui::PushFont(boldFont);
            if (ImGui::Button("Y", buttonSize))
                values.y = resetValue;
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::SetNextItemWidth(widthEach);
            ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
            if (ImGui::IsItemActivated() && onEditBegin)
                onEditBegin();
            if (ImGui::IsItemDeactivatedAfterEdit() && onEditEnd)
                onEditEnd();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.13f, 0.4f, 0.8f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
            ImGui::PushFont(boldFont);
            if (ImGui::Button("Z", buttonSize))
                values.z = resetValue;
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::SetNextItemWidth(widthEach);
            ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
            if (ImGui::IsItemActivated() && onEditBegin)
                onEditBegin();
            if (ImGui::IsItemDeactivatedAfterEdit() && onEditEnd)
                onEditEnd();

            ImGui::PopStyleVar();
            ImGui::EndTable();
        }
        ImGui::PopID();
    }

    void DrawVec2AxisControl(const std::string& label, glm::vec2& values, float resetValue,
                             const std::function<void()>& onEditBegin,
                             const std::function<void()>& onEditEnd)
    {
        ImFont* boldFont = ImGui::GetIO().Fonts->Fonts[0];
        ImGui::PushID(label.c_str());
        if (ImGui::BeginTable(
                    "##Vec2AxisControl", 2,
                    ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn(
                    "Label", ImGuiTableColumnFlags_WidthFixed, InspectorLabelColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(label.c_str());
            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.0f, 0.0f});

            const float lineHeight =
                    GImGui->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            const ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};
            const float widthEach =
                    (ImGui::GetContentRegionAvail().x - 2.0f * buttonSize.x) / 2.0f;

            const auto drawAxis = [&](const char* axisLabel, float& value,
                                      const ImVec4& color, const ImVec4& hoveredColor,
                                      const ImVec4& activeColor)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
                ImGui::PushFont(boldFont);
                if (ImGui::Button(axisLabel, buttonSize))
                {
                    if (onEditBegin)
                        onEditBegin();
                    value = resetValue;
                    if (onEditEnd)
                        onEditEnd();
                }
                ImGui::PopFont();
                ImGui::PopStyleColor(3);

                ImGui::SameLine();
                ImGui::SetNextItemWidth(widthEach);
                std::string inputIdentifier = std::string("##") + axisLabel;
                ImGui::DragFloat(
                        inputIdentifier.c_str(), &value, 0.1f, 0.0f, 0.0f, "%.2f");
                if (ImGui::IsItemActivated() && onEditBegin)
                    onEditBegin();
                if (ImGui::IsItemDeactivatedAfterEdit() && onEditEnd)
                    onEditEnd();
            };

            drawAxis(
                    "X", values.x,
                    ImVec4{0.8f, 0.23f, 0.12f, 1.0f},
                    ImVec4{0.9f, 0.2f, 0.2f, 1.0f},
                    ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
            ImGui::SameLine();
            drawAxis(
                    "Y", values.y,
                    ImVec4{0.12f, 0.7f, 0.2f, 1.0f},
                    ImVec4{0.3f, 0.8f, 0.3f, 1.0f},
                    ImVec4{0.2f, 0.7f, 0.2f, 1.0f});

            ImGui::PopStyleVar();
            ImGui::EndTable();
        }
        ImGui::PopID();
    }

    void DrawStdStringControl(const char* label, std::string& value,
                              const std::function<void()>& onEdited)
    {
        DrawPropertyRow(label, [&]()
        {
            char buffer[256];
            std::memset(buffer, 0, sizeof(buffer));
            std::snprintf(buffer, sizeof(buffer), "%s", value.c_str());

            ImGui::PushItemWidth(-1.0f);
            ImGui::InputText("##Value", buffer, sizeof(buffer));
            ImGui::PopItemWidth();
            if (onEdited && ImGui::IsItemDeactivatedAfterEdit())
            {
                value = buffer;
                onEdited();
            }
            else if (ImGui::IsItemDeactivatedAfterEdit())
            {
                value = buffer;
            }
        });
    }

    void DrawCheckboxControl(const std::string& label, bool& value)
    {
        DrawPropertyRow(label.c_str(), [&]()
        {
            ImGui::Checkbox("##Value", &value);
        });
    }

    void DrawEnumComboControl(const char* label, int& currentIndex, const char* const* labels,
                              int labelCount, const std::function<void(int newIndex)>& onSelectionChanged)
    {
        DrawPropertyRow(label, [&]()
        {
            const char* preview = (currentIndex >= 0 && currentIndex < labelCount)
                ? labels[currentIndex]
                : "None";

            ImGui::PushItemWidth(-1.0f);
            if (ImGui::BeginCombo("##EnumCombo", preview))
            {
                for (int index = 0; index < labelCount; ++index)
                {
                    const bool isSelected = index == currentIndex;
                    if (ImGui::Selectable(labels[index], isSelected))
                    {
                        currentIndex = index;
                        onSelectionChanged(index);
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
        });
    }

    void DrawUInt32Control(const char* label, uint32_t& value, float speed)
    {
        DrawPropertyRow(label, [&]()
        {
            ImGui::PushItemWidth(-1.0f);
            ImGui::DragScalar("##Value", ImGuiDataType_U32, &value, speed, nullptr, nullptr, "%u");
            ImGui::PopItemWidth();
        });
    }

    void DrawIVec2Control(const char* label, glm::ivec2& value, float speed, int minimum, int maximum)
    {
        DrawPropertyRow(label, [&]()
        {
            ImGui::PushItemWidth(-1.0f);
            if (minimum != 0 || maximum != 0)
                ImGui::DragInt2("##Value", &value.x, speed, minimum, maximum);
            else
                ImGui::DragInt2("##Value", &value.x, speed);
            ImGui::PopItemWidth();
        });
    }

    void DrawVec2Control(const char* label, glm::vec2& value, float speed, float minimum, float maximum,
                         const std::function<void()>& onEdited)
    {
        DrawPropertyRow(label, [&]()
        {
            ImGui::PushItemWidth(-1.0f);
            if (minimum != 0.0f || maximum != 0.0f)
                ImGui::DragFloat2("##Value", &value.x, speed, minimum, maximum);
            else
                ImGui::DragFloat2("##Value", &value.x, speed);
            ImGui::PopItemWidth();
            if (onEdited && ImGui::IsItemDeactivatedAfterEdit())
                onEdited();
        });
    }

    void DrawIVec4Control(const char* label, glm::ivec4& value, float speed, int minimum, int maximum,
                          const std::function<void()>& onEdited)
    {
        DrawPropertyRow(label, [&]()
        {
            ImGui::PushItemWidth(-1.0f);
            ImGui::DragInt4("##Value", &value.x, speed, minimum, maximum);
            ImGui::PopItemWidth();
            if (onEdited && ImGui::IsItemDeactivatedAfterEdit())
                onEdited();
        });
    }

    void DrawInputTextControl(const char* label, char* buffer, int bufferSize,
                              const std::function<void()>& onEdited)
    {
        DrawPropertyRow(label, [&]()
        {
            ImGui::PushItemWidth(-1.0f);
            ImGui::InputText("##Value", buffer, static_cast<size_t>(bufferSize));
            ImGui::PopItemWidth();
            if (onEdited && ImGui::IsItemDeactivatedAfterEdit())
                onEdited();
        });
    }

    void DrawReadOnlyTextControl(const char* label, const char* text, const char* tooltipText)
    {
        DrawPropertyRow(label, [&]()
        {
            if (text && text[0] != '\0')
                ImGui::TextUnformatted(text);
            else
                ImGui::TextDisabled("None");
        }, tooltipText);
    }

    void DrawInspectorSectionHeader(const char* title, const char* tooltipText)
    {
        ImGui::Spacing();
        ImGui::SeparatorText(title);
        DrawInspectorTooltipIfHovered(tooltipText);
    }

    void DrawActionButtonRow(const char* label, const std::function<void()>& drawButtons)
    {
        DrawPropertyRow(label, drawButtons);
    }

    bool DrawEditorToggleButton(const char* label, bool isActive, const char* tooltipText)
    {
        return DrawEditorToggleButton(label, isActive, ImVec2(0.0f, 0.0f), tooltipText);
    }

    bool DrawEditorToggleButton(const char* label, bool isActive, const ImVec2& buttonSize,
                                const char* tooltipText)
    {
        constexpr int styleColorCount = 3;
        if (isActive)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
        }

        const bool clicked = ImGui::Button(label, buttonSize);

        if (isActive)
            ImGui::PopStyleColor(styleColorCount);

        DrawInspectorTooltipIfHovered(tooltipText);
        return clicked;
    }

    bool DrawTableFillButton(const char* label, const char* tooltipText)
    {
        const bool clicked = ImGui::Button(label, ImVec2(-FLT_MIN, 0.0f));
        DrawInspectorTooltipIfHovered(tooltipText);
        return clicked;
    }

    void DrawHorizontalButtonPair(const char* pairIdentifier,
                                  const std::function<void()>& drawLeftColumn,
                                  const std::function<void()>& drawRightColumn)
    {
        ImGui::PushID(pairIdentifier);
        if (ImGui::BeginTable("##HorizontalButtonPair", 2, ImGuiTableFlags_SizingStretchSame))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            drawLeftColumn();
            ImGui::TableNextColumn();
            drawRightColumn();
            ImGui::EndTable();
        }
        ImGui::PopID();
    }

    static bool IsImageFileExtension(const std::filesystem::path& filePath)
    {
        std::string extension = filePath.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
        return extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp"
               || extension == ".tga";
    }

    bool AssignTextureFromContentBrowserPayload(const ImGuiPayload* payload, Ref<Texture2D>& texture,
                                                AssetHandle& textureHandle)
    {
        if (!payload)
            return false;

        const wchar_t* relativePathWide = static_cast<const wchar_t*>(payload->Data);
        std::filesystem::path relativePath(relativePathWide);
        std::filesystem::path textureFilePath = Project::GetAssetDirectory() / relativePath;

        if (!std::filesystem::exists(textureFilePath))
            return false;

        if (!IsImageFileExtension(textureFilePath))
            return false;

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return false;

        AssetHandle importedHandle = assetManager->ImportAsset(relativePath);
        if (importedHandle == 0)
            return false;

        textureHandle = importedHandle;
        Ref<Asset> asset = assetManager->GetAsset(importedHandle);
        texture = asset ? std::static_pointer_cast<Texture2D>(asset) : nullptr;
        return texture != nullptr;
    }

    bool AssignSpriteFromContentBrowserPayload(const ImGuiPayload* payload, AssetHandle& spriteAssetHandle)
    {
        if (!payload)
            return false;

        const wchar_t* relativePathWide = static_cast<const wchar_t*>(payload->Data);
        std::filesystem::path relativePath(relativePathWide);
        std::filesystem::path textureFilePath = Project::GetAssetDirectory() / relativePath;

        if (!std::filesystem::exists(textureFilePath) || !IsImageFileExtension(textureFilePath))
            return false;

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return false;

        const AssetHandle textureHandle = assetManager->ImportAsset(relativePath);
        if (textureHandle == 0)
            return false;

        spriteAssetHandle = assetManager->GetDefaultSpriteHandleForTexture(textureHandle);
        return spriteAssetHandle != 0;
    }

    bool AssignAnimationAssetFromContentBrowserPayload(const ImGuiPayload* payload,
                                                       AssetHandle& animationAssetHandle)
    {
        if (!payload)
            return false;

        const wchar_t* relativePathWide = static_cast<const wchar_t*>(payload->Data);
        std::filesystem::path relativePath(relativePathWide);
        std::filesystem::path animationFilePath = Project::GetAssetDirectory() / relativePath;

        if (!std::filesystem::exists(animationFilePath))
            return false;

        std::string extension = animationFilePath.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
        if (extension != ".anim")
            return false;

        auto assetManager = Project::GetAssetManager();
        if (!assetManager)
            return false;

        const AssetHandle importedHandle = assetManager->ImportAsset(relativePath);
        if (importedHandle == 0)
            return false;

        animationAssetHandle = importedHandle;
        return true;
    }

    void DrawObjectReferenceField(const char* label, const char* objectDisplayName, bool hasReference,
                                 const Ref<Texture2D>& previewTexture,
                                 const std::function<void()>& onClear,
                                 const std::function<bool(const ImGuiPayload*)>& onAssignPayload,
                                 const std::function<void()>& onDoubleClick)
    {
        DrawPropertyRow(label, [&]()
        {
            const float frameHeight = ImGui::GetFrameHeight();
            const float verticalPadding = ImGui::GetStyle().FramePadding.y;
            const float slotHeight = frameHeight + verticalPadding * 2.0f;
            const float slotWidth = ImGui::GetContentRegionAvail().x;
            const ImVec2 slotSize(slotWidth, slotHeight);

            const ImVec4 recessedBackground = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
            ImGui::PushStyleColor(ImGuiCol_ChildBg, recessedBackground);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().FrameRounding);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, verticalPadding));

            const ImGuiID slotIdentifier = ImGui::GetID("##ObjectReferenceSlot");
            if (ImGui::BeginChild(slotIdentifier, slotSize, true,
                                  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
            {
                const ImVec2 slotPosition = ImGui::GetWindowPos();
                const ImVec2 childInnerSize = ImGui::GetWindowSize();
                ImDrawList* drawList = ImGui::GetWindowDrawList();

                const char* nameText = "None";
                if (hasReference && objectDisplayName && objectDisplayName[0] != '\0')
                    nameText = objectDisplayName;

                const ImU32 nameColor = hasReference
                    ? ImGui::GetColorU32(ImGuiCol_Text)
                    : ImGui::GetColorU32(ImGuiCol_TextDisabled);

                float nameOffsetX = ImGui::GetStyle().WindowPadding.x;
                const float previewSize = slotHeight - verticalPadding * 2.0f;
                const bool showPreview = previewTexture && previewTexture->GetRendererID() != 0;
                if (showPreview)
                    nameOffsetX += previewSize + 6.0f;

                const ImVec2 nameTextSize = ImGui::CalcTextSize(nameText);
                const float namePositionY =
                    slotPosition.y + (slotSize.y - nameTextSize.y) * 0.5f;
                drawList->AddText(ImVec2(slotPosition.x + nameOffsetX, namePositionY), nameColor, nameText);

                if (showPreview)
                {
                    ImGui::SetCursorPos(ImVec2(ImGui::GetStyle().WindowPadding.x, verticalPadding));
                    DrawEditorTextureImageFull(previewTexture->GetRendererID(),
                                               ImVec2(previewSize, previewSize));
                }

                if (hasReference && onClear)
                {
                    const float clearButtonSize = frameHeight;
                    ImGui::SetCursorPos(ImVec2(childInnerSize.x - clearButtonSize - 6.0f, verticalPadding));
                    if (ImGui::Button("X", ImVec2(clearButtonSize, clearButtonSize)))
                        onClear();
                }

                ImGui::SetNextItemAllowOverlap();
                ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
                ImGui::InvisibleButton("##ObjectReferenceDropZone", childInnerSize);
                if (ImGui::IsItemHovered() && onDoubleClick
                    && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    onDoubleClick();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                        onAssignPayload(payload);
                    ImGui::EndDragDropTarget();
                }
            }
            ImGui::EndChild();

            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor();
        });
    }

    void DrawEditorTextureImageFull(uint64_t textureRendererId, const ImVec2& displaySize)
    {
        const auto& corners = SpriteSheetUtility::FullTextureImGuiUvCorners;
        ImGui::Image((ImTextureID)(intptr_t)textureRendererId, displaySize,
                     ImVec2(corners.TopLeft.x, corners.TopLeft.y),
                     ImVec2(corners.BottomRight.x, corners.BottomRight.y));
    }

    void DrawEditorTextureImageSubRect(uint64_t textureRendererId, const ImVec2& displaySize,
                                       const glm::ivec4& pixelRect, uint32_t textureWidth,
                                       uint32_t textureHeight)
    {
        const auto corners =
                SpriteSheetUtility::PixelRectToImGuiImageUv(pixelRect, textureWidth, textureHeight);
        ImGui::Image((ImTextureID)(intptr_t)textureRendererId, displaySize,
                     ImVec2(corners.TopLeft.x, corners.TopLeft.y),
                     ImVec2(corners.BottomRight.x, corners.BottomRight.y));
    }

} // namespace Himii
