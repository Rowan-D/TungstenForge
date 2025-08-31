#ifndef TUNGSTEN_FORGE_TUNGSTEN_FORGE_HPP
#define TUNGSTEN_FORGE_TUNGSTEN_FORGE_HPP

#include "TungstenUtils/TungstenUtils.hpp"
#include <optional>
#include <filesystem>

namespace wForge
{
    enum class Var
    {
        WorkspacePath,
        ProjectPath,
        EngineDir,
        IntDir,
        BuildDir,
        COUNT
    };

    class TungstenForge
    {
    public:
        TungstenForge();

        inline bool GetIsVarSet(Var type) const { W_ASSERT(type != Var::COUNT, "Var type cannot be set to Var::COUNT!"); return m_varSet[static_cast<std::size_t>(type)]; }
        std::optional<std::filesystem::path> GetVar(Var type) const;
        void SetVar(Var type, const std::filesystem::path& value);
        inline void ClearVar(Var type) { W_ASSERT(type != Var::COUNT, "Var type cannot be set to Var::COUNT!"); m_varSet[static_cast<std::size_t>(type)] = false; }

        inline bool GetIsWorkspacePathSet() const { return GetIsVarSet(Var::WorkspacePath); }
        inline std::optional<std::filesystem::path> GetWorkspacePath() const { return GetVar(Var::WorkspacePath); }
        inline void SetWorkspacePath(const std::filesystem::path& workspacePath) { SetVar(Var::WorkspacePath, workspacePath); }
        inline void ClearWorkspacePath() { ClearVar(Var::WorkspacePath); }

        inline bool GetIsProjectPathSet() const { return GetIsVarSet(Var::ProjectPath); }
        inline std::optional<std::filesystem::path> GetProjectPath() const { return GetVar(Var::ProjectPath); }
        inline void SetProjectPath(const std::filesystem::path& projectPath) { SetVar(Var::ProjectPath, projectPath); }
        inline void ClearProjectPath() { ClearVar(Var::ProjectPath); }

        inline bool GetIsEngineDirSet() const { return GetIsVarSet(Var::EngineDir); }
        inline std::optional<std::filesystem::path> GetEngineDir() const { return GetVar(Var::EngineDir); }
        inline void SetEngineDir(const std::filesystem::path& engineDir) { SetVar(Var::EngineDir, engineDir); }
        inline void ClearEngineDir() { ClearVar(Var::EngineDir); }

        inline bool GetIsIntDirSet() const { return GetIsVarSet(Var::IntDir); }
        inline std::optional<std::filesystem::path> GetIntDir() const { return GetVar(Var::IntDir); }
        inline void SetIntDir(const std::filesystem::path& intDir) { SetVar(Var::IntDir, intDir); }
        inline void ClearIntDir() { ClearVar(Var::IntDir); }

        inline bool GetIsBuildDirSet() const { return GetIsVarSet(Var::BuildDir); }
        inline std::optional<std::filesystem::path> GetBuildDir() const { return GetVar(Var::BuildDir); }
        inline void SetBuildDir(const std::filesystem::path& buildDir) { SetVar(Var::BuildDir, buildDir); }
        inline void ClearBuildDir() { ClearVar(Var::BuildDir); }

        std::optional<std::filesystem::path> GetProjectFilePath(const std::filesystem::path& inputPath);
        bool SetupWorkspace();
        bool BuildProject();

        wUtils::TungstenLogger errorList;

    private:
        bool m_varSet[static_cast<std::size_t>(Var::COUNT)];
        std::filesystem::path m_vars[static_cast<std::size_t>(Var::COUNT)];
    };
}

#endif