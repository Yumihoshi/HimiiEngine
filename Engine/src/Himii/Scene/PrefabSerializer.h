#pragma once

#include "Himii/Scene/Entity.h"
#include "Himii/Scene/Scene.h"

#include <filesystem>
#include <string>

namespace Himii
{
    class PrefabSerializer
    {
    public:
        static bool Save(Entity entity, const std::filesystem::path& filepath,
                         const std::string& prefabName = {});

        static Entity Instantiate(const Ref<Scene>& scene, const std::filesystem::path& filepath);
    };
}
