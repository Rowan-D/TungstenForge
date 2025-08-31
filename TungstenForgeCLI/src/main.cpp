#include "TungstenForge/TungstenForge.hpp"
#include <CLI/CLI.hpp>

int main(int argc, char** argv)
{
    CLI::App app{"TungstenForgeCLI"};

    std::string workspacePath, projectPath, engineDir, intermediateDir, buildDir;

    auto workspaceCommand = app.add_subcommand("workspace", "Setup workspace");
    workspaceCommand->add_option("-W,--workspace", workspacePath, "Path to workspace")->required();
    workspaceCommand->add_option("-P,--project", projectPath, "Path to project")->required();

    auto buildCommand = app.add_subcommand("build", "Build project");
    buildCommand->add_option("-W,--workspace", workspacePath, "Path to workspace");
    buildCommand->add_option("-P,--project", projectPath, "Path to project");
    buildCommand->add_option("-E,--engine", engineDir, "Path to engine directory");
    buildCommand->add_option("-I,--intermediate", intermediateDir, "Path to intermediate directory");
    buildCommand->add_option("-B,--build", buildDir, "final output directory");

    CLI11_PARSE(app, argc, argv);

    wForge::TungstenForge tungstenForge;
    tungstenForge.SetWorkspacePath(workspacePath);
    tungstenForge.SetProjectPath(projectPath);
    tungstenForge.SetEngineDir(engineDir);
    tungstenForge.SetIntDir(intermediateDir);
    tungstenForge.SetBuildDir(buildDir);

    if (*workspaceCommand)
    {
        return 0;
    }
    if (*buildCommand)
    {
        return !tungstenForge.BuildProject();
    }
    std::cout << "TungstenForgeCLI is a command line application for running TungstenForge. Usage:\n\n";

    std::cout << "  wforge workspace -W <workspace-path> -P <project-path>\n";
    std::cout << "  wforge build -W <workspace-path>\n";
    std::cout << "  wforge build -P <project-path> -E <engine-dir> -I <int-dir> -B <build-dir>\n\n";

    std::cout << "Run 'wforge --help' For more information." << std::endl;
    return 0;
}