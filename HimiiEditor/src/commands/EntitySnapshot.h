#pragma once

#include "Engine.h"
#include <string>

namespace Himii
{
    class EntitySnapshot
    {
    public:
        static std::string Serialize(Entity entity);
        static Entity Restore(const Ref<Scene>& scene, const std::string& yaml);
    };
}
