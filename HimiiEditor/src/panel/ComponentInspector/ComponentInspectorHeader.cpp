#include "ComponentInspectorHeader.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Himii
{
    void DrawComponentInspectorHeaderUI(
        ComponentInspectorDrawContext& drawContext,
        const std::string& componentTypeKey,
        const std::string& displayName,
        const Ref<Texture2D>& componentIcon,
        const std::function<void()>& drawComponentContent,
        const std::function<void()>& removeComponentAction)
    {
        const uint64_t entityUUIDValue = static_cast<uint64_t>(drawContext.entity.GetUUID());
        const std::string openStateKey = componentTypeKey + "::IsOpen::" + std::to_string(entityUUIDValue);
        const ImGuiID openStateImGuiId = ImGui::GetID(openStateKey.c_str());

        const std::string popupId =
            std::string("ComponentSetting::") + componentTypeKey + "::" + std::to_string(entityUUIDValue);

        ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
        const float lineHeight = GImGui->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImGui::Separator();

        bool drawComponentRemoved = false;

        ImGui::PushID(componentTypeKey.c_str());

        const int openStateValue = ImGui::GetStateStorage()->GetInt(openStateImGuiId, 1);
        bool open = openStateValue != 0;

        // Calculate start pos
        const ImVec2 cursorPos = ImGui::GetCursorScreenPos(); // Screen Space
        const ImVec2 elementPos = ImGui::GetCursorPos();      // Window Space (for restoring)

        // 1. Draw Background (Lighter color)
        const ImU32 headerColor = ImGui::GetColorU32(ImVec4{0.16f, 0.16f, 0.16f, 1.0f});
        ImGui::GetWindowDrawList()->AddRectFilled(cursorPos,
                                                  ImVec2(cursorPos.x + contentRegionAvailable.x,
                                                         cursorPos.y + lineHeight),
                                                  headerColor);

        // 2. Draw Selectable (Input handling)
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));

        if (ImGui::Selectable("##Header", false, ImGuiSelectableFlags_AllowItemOverlap,
                              ImVec2(contentRegionAvailable.x, lineHeight)))
        {
            open = !open;
            ImGui::GetStateStorage()->SetInt(openStateImGuiId, open ? 1 : 0);
        }

        ImGui::PopStyleColor(3);

        // 3. Draw Arrow & Icon & Text (Overlay)
        // Reset cursor to start of the item to draw on top
        ImGui::SetCursorPos(ImVec2(elementPos.x + ImGui::GetStyle().WindowPadding.x, elementPos.y));

        // Arrow
        ImGui::RenderArrow(ImGui::GetWindowDrawList(),
                            ImVec2(ImGui::GetCursorScreenPos().x,
                                   ImGui::GetCursorScreenPos().y
                                       + (lineHeight - GImGui->FontSize) / 2),
                            ImGui::GetColorU32(ImGuiCol_Text),
                            open ? ImGuiDir_Down : ImGuiDir_Right);

        // Move cursor past arrow
        ImGui::SetCursorPos(ImVec2(elementPos.x + ImGui::GetStyle().WindowPadding.x
                                       + GImGui->FontSize + 4.0f,
                                   elementPos.y));

        // Icon
        if (componentIcon)
        {
            ImGui::Image((ImTextureID)componentIcon->GetRendererID(),
                         ImVec2{lineHeight - 4, lineHeight - 4},
                         ImVec2{0, 1},
                         ImVec2{1, 0});
            ImGui::SameLine();
        }

        // Text alignment fix (center vertically roughly)
        const float textOffsetY = (lineHeight - GImGui->FontSize) / 2.0f;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + textOffsetY);
        ImGui::TextUnformatted(displayName.c_str());

        ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
        ImGui::SetCursorPosY(elementPos.y);

        if (ImGui::Button("+", ImVec2{lineHeight, lineHeight}))
            ImGui::OpenPopup(popupId.c_str());

        ImGui::SetCursorPos(ImVec2(elementPos.x, elementPos.y + lineHeight));

        ImGui::PopStyleVar(); // Pop FramePadding after header and button

        bool clickedRemove = false;
        if (ImGui::BeginPopup(popupId.c_str()))
        {
            if (ImGui::MenuItem("Remove component"))
                clickedRemove = true;
            ImGui::EndPopup();
        }

        if (clickedRemove && removeComponentAction)
        {
            removeComponentAction();
            drawComponentRemoved = true;
        }

        if (open && !drawComponentRemoved && drawComponentContent)
            drawComponentContent();

        ImGui::PopID();
    }
} // namespace Himii

