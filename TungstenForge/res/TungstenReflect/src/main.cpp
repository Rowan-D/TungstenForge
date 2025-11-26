#include "TungstenCore/TungstenCore.hpp"
#include "generated/projectDefines.hpp"
#include TUNGSTEN_PROJECT_INCLUDE_PATH

template <typename... Cs>
constexpr auto ComponentNames(wCore::ComponentList<Cs...>)
{
    return std::array<std::string_view, sizeof...(Cs)>{ Cs::NameView... };
}

constexpr auto Names = ComponentNames(TUNGSTEN_PROJECT_COMPONENT_LIST{});

int main()
{
    for (auto name : Names)
    {
        std::cout << name << '\n';
    }
    return 0;
}