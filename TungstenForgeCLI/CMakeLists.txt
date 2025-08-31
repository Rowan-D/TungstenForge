cmake_minimum_required(VERSION 3.14)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" TUNGSTENFORGECLI_VERSION_RAW)
string(STRIP "${TUNGSTENFORGECLI_VERSION_RAW}" TUNGSTENFORGECLI_VERSION)
string(REPLACE "." ";" TUNGSTENFORGECLI_VERSION_LIST ${TUNGSTENFORGECLI_VERSION})

list(LENGTH TUNGSTENFORGECLI_VERSION_LIST _ver_len)
if(NOT _ver_len EQUAL 3)
    message(FATAL_ERROR "VERSION file contains unknown format; expected: MAJOR.MINOR.PATCH")
endif()

list(GET TUNGSTENFORGECLI_VERSION_LIST 0 TUNGSTENFORGECLI_VERSION_MAJOR)
list(GET TUNGSTENFORGECLI_VERSION_LIST 1 TUNGSTENFORGECLI_VERSION_MINOR)
list(GET TUNGSTENFORGECLI_VERSION_LIST 2 TUNGSTENFORGECLI_VERSION_PATCH)

project(TungstenForgeCLI VERSION ${TUNGSTENFORGECLI_VERSION} LANGUAGES CXX)

message(STATUS "TungstenForgeCLI version: ${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/TungstenForgeCLI/Version.hpp
    @ONLY
)

# Add TungstenForge
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/TungstenForge)
# Add CLI11
set(CLI11_INSTALL OFF CACHE BOOL "Disable CLI11 install target")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/CLI11)

add_executable(TungstenForgeCLI
    src/main.cpp
)

target_compile_features(TungstenForgeCLI PUBLIC cxx_std_20)

target_link_libraries(TungstenForgeCLI PRIVATE
    TungstenForge
    CLI11::CLI11
)

set_target_properties(TungstenForgeCLI PROPERTIES
    OUTPUT_NAME "tungstenforge"
    VERSION ${PROJECT_VERSION}
)

# Installation logic
include(GNUInstallDirs)

message(STATUS "Installing to: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")

install(TARGETS TungstenForgeCLI
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Create wforge -> tungstenforge symlink
install(CODE "
    set(_bin_dir \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\")
    file(MAKE_DIRECTORY \"\${_bin_dir}\")
    message(STATUS \"Creating symlink: \${_bin_dir}/wforge -> tungstenforge\")
    execute_process(
        COMMAND \"${CMAKE_COMMAND}\" -E create_symlink tungstenforge wforge
        WORKING_DIRECTORY \"\${_bin_dir}\"
    )
    if(NOT IS_SYMLINK \"\${_bin_dir}/wforge\")
        message(SEND_ERROR \"Failed to create symlink: \${_bin_dir}/wforge\")
    endif()
")

# Uninstall logic
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)