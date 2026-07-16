#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "Himii/Core/UUID.h"
#include "Himii/Scene/Entity.h"
#include "Himii/Scene/Scene.h"
#include "Himii/Renderer/Texture.h"

namespace Himii
{
    class EditorCommandHistory;

    struct ComponentInspectorDrawContext
    {
        Ref<Scene> scene;
        Entity entity;
        EditorCommandHistory* commandHistory = nullptr;

        // 从组件 key 获取图标（由 SceneHierarchyPanel 提供）
        std::function<Ref<Texture2D>(const std::string& iconKey)> getComponentIcon;

        // 打开联动面板（由 SceneHierarchyPanel 提供）
        std::function<void(AssetHandle)> requestTextureInspector;
        std::function<void(AssetHandle)> requestParticleEmitterEditor;
        std::function<void(AssetHandle)> requestTileMapEditor;
        std::function<void(const std::filesystem::path&)> requestAnimationEditor;
    };
} // namespace Himii

