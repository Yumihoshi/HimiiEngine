#include "Hepch.h"
#include "FileWatcher.h"

namespace Himii
{
    void FileWatcher::Watch(const std::filesystem::path& directory, ChangeCallback onChanged, float debounceSeconds)
    {
        m_Directory = directory;
        m_Callback = std::move(onChanged);
        m_DebounceSeconds = debounceSeconds;
        m_FileTimes.clear();
        m_Dirty = false;
        m_PendingChange = false;
        m_DebounceTimer = 0.0f;
        m_ScanAccumulator = s_ScanInterval;
        m_Initialized = false;

        if (std::filesystem::exists(m_Directory))
            ScanDirectory();
    }

    void FileWatcher::Clear()
    {
        m_Directory.clear();
        m_Callback = nullptr;
        m_FileTimes.clear();
        m_Dirty = false;
        m_PendingChange = false;
        m_Initialized = false;
    }

    void FileWatcher::Update(float deltaSeconds)
    {
        if (m_Directory.empty() || !std::filesystem::exists(m_Directory))
            return;

        m_ScanAccumulator += deltaSeconds;
        if (m_ScanAccumulator >= s_ScanInterval)
        {
            m_ScanAccumulator = 0.0f;
            ScanDirectory();
        }

        if (m_Dirty)
        {
            m_DebounceTimer += deltaSeconds;
            if (m_DebounceTimer >= m_DebounceSeconds)
            {
                m_Dirty = false;
                m_DebounceTimer = 0.0f;
                m_PendingChange = true;
                if (m_Callback)
                    m_Callback();
            }
        }
    }

    void FileWatcher::ScanDirectory()
    {
        bool changed = false;

        std::unordered_map<std::string, std::filesystem::file_time_type> current;

        for (auto& entry : std::filesystem::recursive_directory_iterator(m_Directory))
        {
            if (!entry.is_regular_file())
                continue;

            if (entry.path().extension() != ".cs")
                continue;

            std::string key = entry.path().string();
            auto writeTime = entry.last_write_time();
            current[key] = writeTime;

            auto it = m_FileTimes.find(key);
            if (it == m_FileTimes.end() || it->second != writeTime)
                changed = true;
        }

        if (!m_Initialized)
        {
            m_FileTimes = std::move(current);
            m_Initialized = true;
            return;
        }

        if (m_FileTimes.size() != current.size())
            changed = true;

        m_FileTimes = std::move(current);

        if (changed)
        {
            m_Dirty = true;
            m_DebounceTimer = 0.0f;
        }
    }
}
