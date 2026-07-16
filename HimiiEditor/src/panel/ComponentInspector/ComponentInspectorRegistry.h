#pragma once

#include <string>
#include <vector>

#include "ComponentInspectorDrawContext.h"

namespace Himii
{
    struct ComponentInspectorEntry
    {
        std::string componentTypeKey;
        std::string displayName;
        std::string iconKey;
        int displayOrder = 0;

        bool (*hasComponent)(Entity entity) = nullptr;
        void (*drawInspectorUI)(ComponentInspectorDrawContext& drawContext) = nullptr;
        void (*removeComponent)(Entity entity) = nullptr;
    };

    class ComponentInspectorRegistry
    {
    public:
        static ComponentInspectorRegistry& Get();

        const std::vector<ComponentInspectorEntry>& GetEntries();

        template<typename ComponentType>
        void RegisterComponentInspector(
            const std::string& componentTypeKey,
            const std::string& displayName,
            const std::string& iconKey,
            int displayOrder,
            void (*drawInspectorUI)(ComponentInspectorDrawContext& drawContext))
        {
            ComponentInspectorEntry entry;
            entry.componentTypeKey = componentTypeKey;
            entry.displayName = displayName;
            entry.iconKey = iconKey;
            entry.displayOrder = displayOrder;
            entry.hasComponent = &HasComponentByType<ComponentType>;
            entry.drawInspectorUI = drawInspectorUI;
            entry.removeComponent = &RemoveComponentByType<ComponentType>;

            m_Entries.push_back(entry);
            m_IsSorted = false;
        }

        void DrawAll(ComponentInspectorDrawContext& drawContext);

    private:
        ComponentInspectorRegistry() = default;
        ComponentInspectorRegistry(const ComponentInspectorRegistry&) = delete;
        ComponentInspectorRegistry& operator=(const ComponentInspectorRegistry&) = delete;

        template<typename ComponentType>
        static bool HasComponentByType(Entity entity)
        {
            return entity.HasComponent<ComponentType>();
        }

        template<typename ComponentType>
        static void RemoveComponentByType(Entity entity)
        {
            entity.RemoveComponent<ComponentType>();
        }

    private:
        std::vector<ComponentInspectorEntry> m_Entries;
        bool m_IsSorted = false;
    };
} // namespace Himii

