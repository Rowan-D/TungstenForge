#include "TungstenCore/TungstenCore.hpp"
#include "generated/projectDefines.hpp"
#include TUNGSTEN_PROJECT_INCLUDE_PATH

int main()
{
    wCore::Application app;
    TUNGSTEN_PROJECT_INIT(app.GetComponentManager());
    return 0;
}