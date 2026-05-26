#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    constexpr char kPackFileMagic[4] = {'H', 'P', 'K', '1'};
    constexpr uint32_t kPackFileVersion = 1;

#pragma pack(push, 1)
    struct PackFileHeader
    {
        char Magic[4];
        uint32_t Version;
        uint32_t EntryCount;
        uint64_t IndexOffset;
        uint64_t DataOffset;
    };
#pragma pack(pop)

    struct PackInputEntry
    {
        std::string RelativePath;
        std::vector<uint8_t> Bytes;
    };

    std::string NormalizeRelativePath(std::string relativePath)
    {
        std::replace(relativePath.begin(), relativePath.end(), '\\', '/');
        while (!relativePath.empty() && relativePath.front() == '/')
            relativePath.erase(relativePath.begin());
        return relativePath;
    }

    void CollectFiles(const std::filesystem::path &rootDirectory, const std::string &pathPrefix,
                      std::vector<PackInputEntry> &entries)
    {
        if (!std::filesystem::exists(rootDirectory))
        {
            std::cerr << "Input directory does not exist: " << rootDirectory << std::endl;
            return;
        }

        for (const auto &directoryEntry : std::filesystem::recursive_directory_iterator(rootDirectory))
        {
            if (!directoryEntry.is_regular_file())
                continue;

            const std::string relativePathString =
                    pathPrefix + "/" + std::filesystem::relative(directoryEntry.path(), rootDirectory).string();
            const std::string normalizedPath = NormalizeRelativePath(relativePathString);

            std::ifstream inputStream(directoryEntry.path(), std::ios::binary);
            if (!inputStream)
            {
                std::cerr << "Failed to read: " << directoryEntry.path() << std::endl;
                continue;
            }

            inputStream.seekg(0, std::ios::end);
            const std::streamsize fileSize = inputStream.tellg();
            inputStream.seekg(0, std::ios::beg);

            PackInputEntry entry;
            entry.RelativePath = normalizedPath;
            entry.Bytes.resize(static_cast<size_t>(fileSize));
            inputStream.read(reinterpret_cast<char *>(entry.Bytes.data()), fileSize);
            entries.push_back(std::move(entry));
        }
    }

    bool WritePackFile(const std::filesystem::path &outputPath, const std::vector<PackInputEntry> &entries)
    {
        if (entries.empty())
            return false;

        std::vector<PackInputEntry> sortedEntries = entries;
        for (PackInputEntry &entry : sortedEntries)
            entry.RelativePath = NormalizeRelativePath(std::move(entry.RelativePath));

        std::sort(sortedEntries.begin(), sortedEntries.end(),
                  [](const PackInputEntry &left, const PackInputEntry &right)
                  {
                      return left.RelativePath < right.RelativePath;
                  });

        const std::filesystem::path parentDirectory = outputPath.parent_path();
        if (!parentDirectory.empty())
            std::filesystem::create_directories(parentDirectory);

        std::ofstream outputStream(outputPath, std::ios::binary | std::ios::trunc);
        if (!outputStream)
            return false;

        PackFileHeader header{};
        std::memcpy(header.Magic, kPackFileMagic, 4);
        header.Version = kPackFileVersion;
        header.EntryCount = static_cast<uint32_t>(sortedEntries.size());
        header.IndexOffset = sizeof(PackFileHeader);

        header.DataOffset = header.IndexOffset;
        for (const PackInputEntry &entry : sortedEntries)
        {
            header.DataOffset += sizeof(uint16_t) + entry.RelativePath.size() + sizeof(uint64_t) + sizeof(uint64_t) +
                                 sizeof(uint32_t);
        }

        outputStream.write(reinterpret_cast<const char *>(&header), sizeof(PackFileHeader));

        uint64_t runningDataOffset = 0;
        for (const PackInputEntry &entry : sortedEntries)
        {
            const uint16_t pathLength = static_cast<uint16_t>(entry.RelativePath.size());
            const uint64_t dataSize = entry.Bytes.size();
            const uint32_t flags = 0;

            outputStream.write(reinterpret_cast<const char *>(&pathLength), sizeof(uint16_t));
            outputStream.write(entry.RelativePath.data(), static_cast<std::streamsize>(entry.RelativePath.size()));
            outputStream.write(reinterpret_cast<const char *>(&runningDataOffset), sizeof(uint64_t));
            outputStream.write(reinterpret_cast<const char *>(&dataSize), sizeof(uint64_t));
            outputStream.write(reinterpret_cast<const char *>(&flags), sizeof(uint32_t));

            runningDataOffset += dataSize;
        }

        for (const PackInputEntry &entry : sortedEntries)
        {
            if (!entry.Bytes.empty())
                outputStream.write(reinterpret_cast<const char *>(entry.Bytes.data()),
                                   static_cast<std::streamsize>(entry.Bytes.size()));
        }

        return static_cast<bool>(outputStream);
    }
}

int main(int argc, char **argv)
{
    std::filesystem::path outputPath;
    std::vector<PackInputEntry> entries;

    for (int argumentIndex = 1; argumentIndex < argc; ++argumentIndex)
    {
        const std::string argument = argv[argumentIndex];
        if (argument == "--output" && argumentIndex + 1 < argc)
        {
            outputPath = argv[++argumentIndex];
        }
        else if (argument == "--root" && argumentIndex + 2 < argc)
        {
            const std::filesystem::path rootDirectory = argv[++argumentIndex];
            const std::string pathPrefix = argv[++argumentIndex];
            CollectFiles(rootDirectory, pathPrefix, entries);
        }
        else
        {
            std::cerr << "Usage: ResourcePacker --root <dir> <prefix> [--root <dir> <prefix> ...] --output <file.hpck>"
                      << std::endl;
            return 1;
        }
    }

    if (outputPath.empty() || entries.empty())
    {
        std::cerr << "ResourcePacker: missing --output or no input files." << std::endl;
        return 1;
    }

    if (!WritePackFile(outputPath, entries))
    {
        std::cerr << "ResourcePacker: failed to write " << outputPath << std::endl;
        return 1;
    }

    std::cout << "ResourcePacker: wrote " << entries.size() << " entries to " << outputPath << std::endl;
    return 0;
}
