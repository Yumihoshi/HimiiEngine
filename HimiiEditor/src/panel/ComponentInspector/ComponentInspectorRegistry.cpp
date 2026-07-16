#include "ComponentInspectorRegistry.h"

#include <algorithm>

namespace Himii
{
    ComponentInspectorRegistry& ComponentInspectorRegistry::Get()
    {
        static ComponentInspectorRegistry registry;
        return registry;
    }

    const std::vector<ComponentInspectorEntry>& ComponentInspectorRegistry::GetEntries()
    {
        if (!m_IsSorted)
        {
            std::sort(m_Entries.begin(), m_Entries.end(),
                      [](const ComponentInspectorEntry& left, const ComponentInspectorEntry& right)
                      {
                          return left.displayOrder < right.displayOrder;
                      });
            m_IsSorted = true;
        }

        return m_Entries;
    }

    void ComponentInspectorRegistry::DrawAll(ComponentInspectorDrawContext& drawContext)
    {
        for (const ComponentInspectorEntry& entry : GetEntries())
        {
            if (entry.hasComponent && entry.hasComponent(drawContext.entity))
            {
                if (entry.drawInspectorUI)
                    entry.drawInspectorUI(drawContext);
            }
        }
    }
} // namespace Himii

