#pragma once

#include "Engine.h"
#include <string>

namespace Himii
{
    class EntitySnapshot
    {
    public:
        static std::string Serialize(Entity entity);
        static std::string SerializeSubtree(const Ref<Scene>& scene, Entity rootEntity);
        static Entity Restore(const Ref<Scene>& scene, const std::string& yaml);
        static void RestoreSubtree(const Ref<Scene>& scene, const std::string& yaml);
    };
}
