set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.16)

# set the project name and version
project(tech-game-state VERSION 1.0.0 DESCRIPTION "Botball Game State library of team 'TechSupport'")

# define the source and header files
file(GLOB SRC_FILES "src/*.cpp")
file(GLOB INCLUDE_FILES "include/*.h" "lib/*.hpp")

add_library(${PROJECT_NAME} STATIC
        include/gamestate.h)

add_executable(bot_a "bot_a.cpp")
add_executable(bot_b "bot_b.cpp")
set_target_properties(bot_a PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
set_target_properties(bot_b PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

target_link_libraries(tech-game-state PRIVATE spdlog::spdlog fmt::fmt)
target_link_libraries(bot_a PRIVATE tech-game-state)
target_link_libraries(bot_b PRIVATE tech-game-state)

# specify the C++ standard
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Set the version property.
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

# Set the shared object version property to the project's major version.
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION_MAJOR})

set(public_headers ${INCLUDE_FILES})

target_sources(${PROJECT_NAME}
        PRIVATE
        ${SRC_FILES}
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# without it public headers won't get installed
set(public_headers
        ${INCLUDE_FILES}
)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${public_headers}")

# set the compiler and linker flags
target_compile_definitions(${PROJECT_NAME} PUBLIC NDEBUG)
target_compile_options(${PROJECT_NAME} PRIVATE -O3 -Wall -Wextra -Wno-psabi)

# needed for installing the library
include(GNUInstallDirs)

# install the target and create export-set
install(TARGETS ${PROJECT_NAME}
        EXPORT "${PROJECT_NAME}Targets"
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME} # include/techlib
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # include
)


# set the namespace
set(namespace "tech-gamestate")

# generate and install export file
install(EXPORT "${PROJECT_NAME}Targets"
        FILE "${PROJECT_NAME}Targets.cmake"
        NAMESPACE ${namespace}::
        DESTINATION cmake
)

include(CMakePackageConfigHelpers)
