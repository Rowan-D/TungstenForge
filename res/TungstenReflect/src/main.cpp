#include "TungstenCore/TungstenCore.hpp"
#include "generated/projectDefines.hpp"
#include TUNGSTEN_PROJECT_INCLUDE_PATH

#include <filesystem>
#include <fstream>

int main(int argc, char** argv)
{
    wUtils::TungstenLogger errorList;
    std::filesystem::path outputPath = "ComponentTypes.txt";

    for (int argIndex = 1; argIndex < argc - 1; ++argIndex) {
        if (std::string_view(argv[argIndex]) == "-o") {
            outputPath = argv[argIndex + 1];
            break;
        }
    }

    wCore::Application app;
    wCore::ComponentSetup& componentSetup = app.GetComponentSystem().GetComponentSetup();
    TUNGSTEN_PROJECT_INIT(componentSetup);

    std::ofstream out(outputPath);

    if (!out)
    {
        W_LOG_ERROR(errorList, "Failed to open output file: {}", outputPath.string());
        return 1;
    }

    for (wIndex id = wCore::ComponentSetup::IDStart; id <= componentSetup.GetComponentTypeCount(); ++id)
    {
        out << componentSetup.GetComponentTypeNameFromID(id) << '\n';
    }

    if (!out)
    {
        W_LOG_ERROR(errorList, "Failed while writing to output file: {}", outputPath.string());
        return 1;
    }

    W_LOG_INFO(errorList, "Component Type List written to: {}", outputPath.string());
    return 0;
}