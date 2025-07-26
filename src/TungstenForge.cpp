#include <array>
#include <span>

#include <ryml.hpp>
#include <ryml_std.hpp>

#include "TungstenForge/TungstenForge.hpp"
#include "TungstenUtils/ReadFile.hpp"
#include "TungstenUtils/WriteFile.hpp"
#include "TungstenUtils/FindAndReplace.hpp"

namespace wForge
{
    static bool RenderTemplateFile(const std::filesystem::path& templateFile, const std::filesystem::path& outputPath, std::span<const std::pair<std::string_view, std::string_view>> replacements)
    {
        std::optional<std::string> templateStr = wUtils::ReadFile(templateFile);
        if (!templateStr)
        {
            return false;
        }
        for (const auto& [from, to] : replacements)
        {
            W_DEBUG_LOG_INFO("{}", *templateStr);
            W_DEBUG_LOG_INFO("from: {} to: {}", from, to);
            wUtils::FindAndReplaceAll(*templateStr, from, to);
        }
        W_DEBUG_LOG_INFO("{}", *templateStr);
        return wUtils::WriteFile(outputPath, *templateStr);
    }

    TungstenForge::TungstenForge()
        : errorList()
    {

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

    bool TungstenForge::BuildProject(const std::filesystem::path& projectPath, const std::filesystem::path& tungstenCoreSourceDir, const std::filesystem::path& outputIntDir, const std::filesystem::path& outputBuildDir)
    {
        namespace fs = std::filesystem;

        W_LOG_INFO(errorList, "Build called.");
        W_LOG_INFO(errorList, "Project path: {}", projectPath.string());
        W_LOG_INFO(errorList, "Output Intermediate Directory: {}", outputIntDir.string());
        W_LOG_INFO(errorList, "Output Build Directory: {}", outputBuildDir.string());

        try
        {
            const std::optional<fs::path> projectFilePath = GetProjectFilePath(projectPath);
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
            const c4::csubstr initCodeVal = root["initCode"].val();
            const std::string_view projectName(projectNameVal.str, projectNameVal.len);
            const std::string_view include(includeVal.str, includeVal.len);
            const std::string_view initCode(initCodeVal.str, initCodeVal.len);
            const std::string_view executableName = projectName;
            const std::string executableTargetName = fmt::format("{}Runtime", executableName);

            fs::create_directory(outputIntDir / "Intermediate");

            fs::copy("res/TungstenRuntime", outputIntDir / "Intermediate/TungstenRuntime", fs::copy_options::recursive | fs::copy_options::overwrite_existing);

            fs::create_directory(outputIntDir / "Intermediate/build");
            fs::create_directory(outputBuildDir / "Build");

            const fs::path projectDir = projectFilePath->parent_path();
            const std::string projectDirStr = fs::weakly_canonical(projectDir).string();
            const std::string tungstenCoreSourceDirStr = fs::weakly_canonical(tungstenCoreSourceDir).string();

            const std::string tungstenProjectBinaryDir = fmt::format("${{CMAKE_BINARY_DIR}}/{}", projectName);
            const std::array<std::pair<std::string_view, std::string_view>, 3> wProjectCMakeListsReplacements = {{
                { "@TUNGSTEN_CORE_SOURCE_DIR@", tungstenCoreSourceDirStr },
                { "@TUNGSTEN_PROJECT_SOURCE_DIR@", projectDirStr },
                { "@TUNGSTEN_PROJECT_BINARY_DIR@", tungstenProjectBinaryDir }
            }};

            const std::array<std::pair<std::string_view, std::string_view>, 3> wRuntimeMakeListsReplacements = {{
                { "@PROJECT_NAME@", projectName },
                { "@EXECUTABLE_TARGET_NAME@", executableTargetName },
                { "@EXECUTABLE_NAME@", executableName }
            }};

            const std::array<std::pair<std::string_view, std::string_view>, 2> wRuntimeGeneratedProjectDefinesReplacements = {{
                { "@TUNGSTEN_PROJECT_INCLUDE_PATH@", include },
                { "@TUNGSTEN_PROJECT_INIT@", initCode },
            }};

            RenderTemplateFile("res/Templates/TungstenProjectCMakeListsTemplate.txt", outputIntDir / "Intermediate/CMakeLists.txt", wProjectCMakeListsReplacements);
            RenderTemplateFile("res/Templates/TungstenRuntimeCMakeListsTemplate.txt", outputIntDir / "Intermediate/TungstenRuntime/CMakeLists.txt", wRuntimeMakeListsReplacements);
            RenderTemplateFile("res/Templates/TungstenRuntimeProjectDefines.in.hpp", outputIntDir / "Intermediate/TungstenRuntime/src/generated/projectDefines.hpp", wRuntimeMakeListsReplacements);
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