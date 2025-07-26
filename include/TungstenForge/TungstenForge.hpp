#ifndef TUNGSTEN_FORGE_TUNGSTEN_FORGE_HPP
#define TUNGSTEN_FORGE_TUNGSTEN_FORGE_HPP

#include "TungstenUtils/TungstenUtils.hpp"
#include <optional>
#include <filesystem>

namespace wForge
{
    class TungstenForge
    {
    public:
        TungstenForge();

        std::optional<std::filesystem::path> GetProjectFilePath(const std::filesystem::path& inputPath);
        bool BuildProject(const std::filesystem::path& projectPath, const std::filesystem::path& tungstenCoreSourceDir, const std::filesystem::path& outputIntDir, std::filesystem::path& outputBuildDir);

        wUtils::TungstenLogger errorList;

    private:
    };
}

#endif