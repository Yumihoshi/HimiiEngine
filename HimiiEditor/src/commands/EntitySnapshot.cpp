#include "EntitySnapshot.h"
#include "Himii/Scene/SceneSerializer.h"
#include <yaml-cpp/yaml.h>

namespace Himii
{
    std::string EntitySnapshot::Serialize(Entity entity)
    {
        YAML::Emitter output;
        SceneSerializer::SerializeEntity(output, entity);
        return std::string(output.c_str());
    }

    Entity EntitySnapshot::Restore(const Ref<Scene>& scene, const std::string& yaml)
    {
        YAML::Node node = YAML::Load(yaml);
        if (!node["Entity"])
            return {};

        UUID entityIdentifier = node["Entity"].as<uint64_t>();
        Entity existingEntity = scene->GetEntityByUUID(entityIdentifier);
        if (existingEntity)
            scene->DestroyEntity(existingEntity);

        SceneSerializer::DeserializeEntity(node, scene);
        return scene->GetEntityByUUID(entityIdentifier);
    }
}
