cmake_minimum_required(VERSION 3.14)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" TUNGSTENFORGE_VERSION_RAW)
string(STRIP "${TUNGSTENFORGE_VERSION_RAW}" TUNGSTENFORGE_VERSION)
string(REPLACE "." ";" TUNGSTENFORGE_VERSION_LIST ${TUNGSTENFORGE_VERSION})

list(LENGTH TUNGSTENFORGE_VERSION_LIST _ver_len)
if(NOT _ver_len EQUAL 3)
    message(FATAL_ERROR "VERSION file contains unknown format; expected: MAJOR.MINOR.PATCH")
endif()

list(GET TUNGSTENFORGE_VERSION_LIST 0 TUNGSTENFORGE_VERSION_MAJOR)
list(GET TUNGSTENFORGE_VERSION_LIST 1 TUNGSTENFORGE_VERSION_MINOR)
list(GET TUNGSTENFORGE_VERSION_LIST 2 TUNGSTENFORGE_VERSION_PATCH)

project(TungstenForge VERSION ${TUNGSTENFORGE_VERSION} LANGUAGES CXX)

message(STATUS "TungstenForge version: ${PROJECT_VERSION}")

option(TUNGSTENFORGE_INSTALL_LIBRARY "Install library, headers, and CMake config" OFF)
option(TUNGSTENFORGE_INSTALL_RES "Install resource directory (wforge/res/)" ON)
option(TUNGSTENFORGE_CLEAN_EMPTY_DIRS "Recursively clean up empty parent directories during uninstall" OFF)
option(TUNGSTENFORGE_UNINSTALL_DRY_RUN "Show what would be removed, without removing anything" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/TungstenForge/Version.hpp
    @ONLY
)

include(GNUInstallDirs)

set(TUNGSTENFORGE_RESOURCE_PATH "${CMAKE_INSTALL_FULL_DATADIR}/wforge/res")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/TungstenForge/Config.hpp
    @ONLY
)

# Add Rapid YAML
set(RYML_INSTALL OFF CACHE BOOL "Disable install target for rapidyaml")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/rapidlyaml)
# Add TungstenUtils
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/TungstenUtils)

add_library(TungstenForge STATIC
    include/TungstenForge/TungstenForge.hpp
    src/TungstenForge.cpp
)

target_include_directories(TungstenForge PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(TungstenForge PUBLIC cxx_std_20)

target_link_libraries(TungstenForge
    PUBLIC TungstenUtils
    PRIVATE ryml
)

set_target_properties(TungstenForge PROPERTIES
    VERSION ${PROJECT_VERSION}
)

# Installation logic
if(TUNGSTENFORGE_ENABLE_INSTALL)
    install(TARGETS TungstenForge
            EXPORT TungstenForgeTargets
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/include/TungstenForge/Version.hpp"
        "${CMAKE_CURRENT_BINARY_DIR}/include/TungstenForge/Config.hpp"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/TungstenForge
    )

    install(EXPORT TungstenForgeTargets
            FILE TungstenForgeTargets.cmake
            NAMESPACE Tungsten::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TungstenForge)

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/TungstenForgeConfigVersion.cmake"
        VERSION ${TUNGSTENFORGE_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/CMake/TungstenForgeConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/TungstenForgeConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TungstenForge
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/TungstenForgeConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/TungstenForgeConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TungstenForge
    )
endif()

# Install runtime resources if enabled
if(TUNGSTENFORGE_INSTALL_RES)
    install(DIRECTORY res/
            DESTINATION ${TUNGSTENFORGE_RESOURCE_PATH})
endif()

# Uninstall logic. Only defined if this is the top-level project.
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/CMake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY
    )

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    )
endif()