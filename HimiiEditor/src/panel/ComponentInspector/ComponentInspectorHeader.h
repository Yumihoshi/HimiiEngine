#pragma once

#include <functional>
#include <string>

#include "ComponentInspectorDrawContext.h"

namespace Himii
{
    void DrawComponentInspectorHeaderUI(
        ComponentInspectorDrawContext& drawContext,
        const std::string& componentTypeKey,
        const std::string& displayName,
        const Ref<Texture2D>& componentIcon,
        const std::function<void()>& drawComponentContent,
        const std::function<void()>& removeComponentAction);
} // namespace Himii

