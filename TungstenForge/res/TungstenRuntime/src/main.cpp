#include "TungstenCore/TungstenCore.hpp"
#include "generated/projectDefines.hpp"
#include TUNGSTEN_PROJECT_INCLUDE_PATH

class ComponentSetup
{
    struct ComponentType
    {

    }

    template<typename T>

    static void CreateComponent()
    {

    }
}

template <typename... Cs>
constexpr ComponentSetup SetupComponents(wCore::ComponentList<Cs...>)
{
}

constexpr ComponentSetup setup = SetupComponents(TUNGSTEN_PROJECT_COMPONENT_LIST{});
int main()
{
    wCore::Application app(setup);
    return app.Run().exitCode;
}