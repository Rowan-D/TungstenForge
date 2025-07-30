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

        bool GetIsWorkspacePathSet() const;
        std::optional<std::filesystem::path> GetWorkspacePath() const;
        void SetWorkspacePath(const std::filesystem::path& workspacePath);
        void ClearWorkspacePath();

        bool GetIsProjectPathSet() const;
        std::optional<std::filesystem::path> GetProjectPath() const;
        void SetProjectPath(const std::filesystem::path& projectPath);
        void ClearProjectPath();

        std::optional<std::filesystem::path> GetProjectFilePath(const std::filesystem::path& inputPath);
        bool BuildProject(const std::filesystem::path& projectPath, const std::filesystem::path& tungstenCoreSourceDir, const std::filesystem::path& outputIntDir, const std::filesystem::path& outputBuildDir);

        wUtils::TungstenLogger errorList;

    private:
    };
}

#endif