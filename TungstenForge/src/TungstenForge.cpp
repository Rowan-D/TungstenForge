#include <array>
#include <span>
#include <cstdlib>

#include <ryml.hpp>
#include <ryml_std.hpp>

#include "TungstenForge/TungstenForge.hpp"
#include "TungstenUtils/ReadFile.hpp"
#include "TungstenUtils/WriteFile.hpp"
#include "TungstenUtils/FindAndReplace.hpp"
#include "TungstenForge/Config.hpp"

namespace wForge
{
    static constexpr std::size_t WorkspacePathVarIndex = static_cast<std::size_t>(Var::WorkspacePath);
    static constexpr std::size_t ProjectPathVarIndex = static_cast<std::size_t>(Var::ProjectPath);
    static constexpr std::size_t EngineDirVarIndex = static_cast<std::size_t>(Var::EngineDir);
    static constexpr std::size_t IntDirVarIndex = static_cast<std::size_t>(Var::IntDir);
    static constexpr std::size_t BuildDirVarIndex = static_cast<std::size_t>(Var::BuildDir);
    static constexpr std::size_t VarCount = static_cast<std::size_t>(Var::COUNT);

    static bool RenderTemplateFile(const std::filesystem::path& templateFile, const std::filesystem::path& outputPath, std::span<const std::pair<std::string_view, std::string_view>> replacements)
    {
        std::optional<std::string> templateStr = wUtils::ReadFile(templateFile);
        if (!templateStr)
        {
            return false;
        }
        for (const auto& [from, to] : replacements)
        {
            wUtils::FindAndReplaceAll(*templateStr, from, to);
        }
        return wUtils::WriteFile(outputPath, *templateStr);
    }

    TungstenForge::TungstenForge()
        : errorList()
    {
        for (bool& varSet : m_varSet)
        {
            varSet = false;
        }
    }

    std::optional<std::filesystem::path> TungstenForge::GetVar(Var type) const
    {
        W_ASSERT(type != Var::COUNT, "Var type cannot be set to Var::COUNT");
        const std::size_t varIndex = static_cast<std::size_t>(type);
        if (m_varSet[varIndex])
        {
            return m_vars[varIndex];
        }
        return std::nullopt;
    }

    void TungstenForge::SetVar(Var type, const std::filesystem::path& value)
    {
        W_ASSERT(type != Var::COUNT, "Var type cannot be set to Var::COUNT");
        const std::size_t varIndex = static_cast<std::size_t>(type);
        m_varSet[varIndex] = true;
        m_vars[varIndex] = value;
    }

    std::optional<std::filesystem::path> TungstenForge::GetProjectFilePath(const std::filesystem::path& inputPath)
    {
        namespace fs = std::filesystem;

        try
        {
            if (fs::exists(inputPath))
            {
                if (fs::is_regular_file(inputPath))
                {
                    return inputPath;
                }

                if (!fs::is_directory(inputPath))
                {
                    W_LOG_ERROR(errorList, "Locating Project File Invalid Path Error: The provided path is neither a project file or directory!");
                    return std::nullopt;
                }

                std::vector<fs::path> wProjFiles;
                for (const auto& entry : fs::directory_iterator(inputPath))
                {
                    if (entry.is_regular_file() && entry.path().extension() == ".wproj")
                    {
                        wProjFiles.push_back(entry.path());
                    }
                }

                if (wProjFiles.empty())
                {
                    W_LOG_ERROR(errorList, "Project File Not Found Error: Tungsten Project File (.wproj) not found!");
                    return std::nullopt;
                }
                else if (wProjFiles.size() > 1)
                {
                    std::string wProjFilesList;
                    for (fs::path wProjFile : wProjFiles)
                    {
                        wProjFilesList += '\n' + wProjFile.filename().string();
                    }
                    W_LOG_ERROR(errorList, "Multiple Project File Error: There should be no more than one Tungsten Project File (.wproj) in the project directory. {} were found!{}", wProjFiles.size(), wProjFilesList);
                    return {};
                }
                else
                {
                    return wProjFiles.front();
                }
            }
            else
            {
                W_LOG_ERROR(errorList, "Locating Project File Invalid Path Error: project file or directory dose not exist!");
                return {};
            }
        }
        catch (const fs::filesystem_error& e)
        {
            W_LOG_ERROR(errorList, "Locating Project File Filesystem Error: {}", e.what());
            return std::nullopt;
        }
        catch (const std::exception& e)
        {
            W_LOG_ERROR(errorList, "Locating Project File General Error: {}", e.what());
            return std::nullopt;
        }
    }

    bool TungstenForge::SetupWorkspace()
    {
        return false;
    }

    bool TungstenForge::BuildProject()
    {
        namespace fs = std::filesystem;

        W_LOG_INFO(errorList, "Build called.");

        const fs::path tungstenCoreSourceDir = m_vars[EngineDirVarIndex] / "TungstenCore";

        W_LOG_INFO(errorList, "projectPath: {}", m_vars[ProjectPathVarIndex].string());
        W_LOG_INFO(errorList, "wCoreSourceDir: {}", tungstenCoreSourceDir.string());
        W_LOG_INFO(errorList, "intDir: {}", m_vars[IntDirVarIndex].string());
        W_LOG_INFO(errorList, "buildDir: {}", m_vars[BuildDirVarIndex].string());

        try
        {
            const std::optional<fs::path> projectFilePath = GetProjectFilePath(m_vars[ProjectPathVarIndex]);
            if (!projectFilePath)
            {
                W_LOG_ERROR(errorList, "Could not get project file path!");
                return false;
            }
            std::optional<std::string> projectFile = wUtils::ReadFile(*projectFilePath);
            if (!projectFile)
            {
                W_LOG_ERROR(errorList, "Could not read project file!");
                return false;
            }
            std::string yamlStr = std::move(*projectFile);
            ryml::Tree tree = ryml::parse_in_place(ryml::to_substr(yamlStr));
            ryml::NodeRef root = tree.rootref();

            if (!root.has_child("projectName"))
            {
                W_LOG_ERROR(errorList, "{} has no \"projectName\"", projectFilePath->string());
                return false;
            }

            const c4::csubstr projectNameVal = root["projectName"].val();
            const c4::csubstr includeVal = root["include"].val();
            const c4::csubstr componentListVal = root["componentList"].val();
            const std::string_view projectName(projectNameVal.str, projectNameVal.len);
            const std::string_view include(includeVal.str, includeVal.len);
            const std::string_view omponentList(componentListVal.str, componentListVal.len);
            const std::string_view executableName = projectName;
            const std::string executableTargetName = std::string(executableName) + "Runtime";

            fs::create_directory(m_vars[IntDirVarIndex]);

            const std::filesystem::path tungstenResDir = TUNGSTENFORGE_RESOURCE_PATH;

            fs::copy(tungstenResDir / "TungstenReflect", m_vars[IntDirVarIndex] / "TungstenReflect", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            fs::create_directory(m_vars[IntDirVarIndex] / "TungstenReflect/src/generated");

            fs::copy(tungstenResDir / "TungstenRuntime", m_vars[IntDirVarIndex] / "TungstenRuntime", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            fs::create_directory(m_vars[IntDirVarIndex] / "TungstenRuntime/src/generated");


            const fs::path projectDir = projectFilePath->parent_path();
            const std::string projectDirStr = fs::weakly_canonical(projectDir).string();
            const std::string tungstenCoreSourceDirStr = fs::weakly_canonical(tungstenCoreSourceDir).string();

            const std::string tungstenProjectBinaryDir = fmt::format("${{CMAKE_BINARY_DIR}}/{}", projectName);
            const std::array<std::pair<std::string_view, std::string_view>, 3> wProjectCMakeListsReplacements = {{
                { "@TUNGSTEN_CORE_SOURCE_DIR@", tungstenCoreSourceDirStr },
                { "@TUNGSTEN_PROJECT_SOURCE_DIR@", projectDirStr },
                { "@TUNGSTEN_PROJECT_BINARY_DIR@", tungstenProjectBinaryDir }
            }};

            const std::array<std::pair<std::string_view, std::string_view>, 3> wReflectCMakeListsReplacements = {{
                { "@PROJECT_NAME@", projectName },
            }};

            const std::array<std::pair<std::string_view, std::string_view>, 2> wReflectAndRuntimeGeneratedProjectDefinesReplacements = {{
                { "@TUNGSTEN_PROJECT_INCLUDE_PATH@", include },
                { "@TUNGSTEN_PROJECT_COMPONENT_LIST@", omponentList }
            }};

            const std::array<std::pair<std::string_view, std::string_view>, 3> wRuntimeCMakeListsReplacements = {{
                { "@PROJECT_NAME@", projectName },
                { "@EXECUTABLE_TARGET_NAME@", executableTargetName },
                { "@EXECUTABLE_NAME@", executableName }
            }};

            RenderTemplateFile(tungstenResDir / "Templates/TungstenProjectCMakeListsTemplate.txt", m_vars[IntDirVarIndex] / "CMakeLists.txt", wProjectCMakeListsReplacements);
            RenderTemplateFile(tungstenResDir / "Templates/TungstenReflectCMakeListsTemplate.txt", m_vars[IntDirVarIndex] / "TungstenReflect/CMakeLists.txt", wReflectCMakeListsReplacements);
            RenderTemplateFile(tungstenResDir / "Templates/TungstenReflectProjectDefines.in.hpp", m_vars[IntDirVarIndex] / "TungstenReflect/src/generated/projectDefines.hpp", wReflectAndRuntimeGeneratedProjectDefinesReplacements);
            RenderTemplateFile(tungstenResDir / "Templates/TungstenRuntimeCMakeListsTemplate.txt", m_vars[IntDirVarIndex] / "TungstenRuntime/CMakeLists.txt", wRuntimeCMakeListsReplacements);
            RenderTemplateFile(tungstenResDir / "Templates/TungstenRuntimeProjectDefines.in.hpp", m_vars[IntDirVarIndex] / "TungstenRuntime/src/generated/projectDefines.hpp", wReflectAndRuntimeGeneratedProjectDefinesReplacements);

            const fs::path buildDir = fs::absolute(m_vars[IntDirVarIndex] / "build");
            fs::create_directory(buildDir);
            const std::string buildDirStr = buildDir.string();

            W_LOG_INFO(errorList, "Configuring CMake project");
            if (std::system(fmt::format("cmake -S \"{}\" -B \"{}\"", fs::absolute(m_vars[IntDirVarIndex]).string(), buildDirStr).c_str()))
            {
                W_LOG_ERROR(errorList, "CMake configuration failed.");
                return false;
            }

            W_LOG_INFO(errorList, "Building project");
            if (std::system(fmt::format("cmake --build \"{}\"", buildDirStr).c_str()))
            {
                W_LOG_ERROR(errorList, "Build failed.");
                return false;
            }

            const std::string wReflectPathStr = fs::absolute(buildDir / "TungstenReflect/TungstenReflect").string();
            const std::string wReflectOutputPathStr = fs::absolute(m_vars[IntDirVarIndex] / "ComponentTypes.txt").string();

            W_LOG_INFO(errorList, "Running TungstenReflect");
            if (std::system(fmt::format("\"{}\" -o \"{}\"", wReflectPathStr, wReflectOutputPathStr).c_str()))
            {
                W_LOG_ERROR(errorList, "TungstenReflect failed.");
                return false;
            }

            fs::create_directory(m_vars[BuildDirVarIndex]);
            fs::copy_file(buildDir / "TungstenRuntime" / executableName, m_vars[BuildDirVarIndex] / executableName, fs::copy_options::overwrite_existing);
            W_LOG_INFO(errorList, "Copied final binary to distribution folder.");

            

            return true;
        }
        catch (const fs::filesystem_error& e)
        {
            W_LOG_ERROR(errorList, "Building Project Filesystem Error: {}", e.what());
            return false;
        }
        catch (const std::exception& e)
        {
            W_LOG_ERROR(errorList, "Building Project General Error: {}", e.what());
            return false;
        }
    }
}