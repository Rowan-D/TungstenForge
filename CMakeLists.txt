cmake_minimum_required(VERSION 3.14)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" TUNGSTENFORGE_VERSION_RAW)
string(STRIP "${TUNGSTENFORGE_VERSION_RAW}" TUNGSTENFORGE_VERSION)
string(REPLACE "." ";" TUNGSTENFORGE_VERSION_LIST ${TUNGSTENFORGE_VERSION})

list(LENGTH TUNGSTENFORGE_VERSION_LIST _ver_len)
if(NOT _ver_len EQUAL 3)
    message(FATAL_ERROR "VERSION file contains unknown format expected: MAJOR.MINOR.PATCH")
endif()

list(GET TUNGSTENFORGE_VERSION_LIST 0 TUNGSTENFORGE_VERSION_MAJOR)
list(GET TUNGSTENFORGE_VERSION_LIST 1 TUNGSTENFORGE_VERSION_MINOR)
list(GET TUNGSTENFORGE_VERSION_LIST 2 TUNGSTENFORGE_VERSION_PATCH)

project(TungstenForge VERSION ${TUNGSTENFORGE_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

# Add Rapid YAML
add_subdirectory(external/rapidyaml)
# Add TungstenForge
add_subdirectory(external/TungstenForge)

add_library(TungstenForge
    include/TungstenForge/TungstenForge.hpp
    src/TungstenForge.cpp
)

target_include_directories(TungstenForge PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TungstenForge PRIVATE ryml)
target_link_libraries(TungstenForge PUBLIC TungstenUtils)