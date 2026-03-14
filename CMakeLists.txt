cmake_minimum_required(VERSION 3.16)
project(botball-gamestate VERSION 1.0 LANGUAGES CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Source files
set(SOURCES
        include/GameState.h
        impl/Client.cpp
        impl/GameState.cpp
        impl/Phase.cpp
        impl/PhaseState.cpp
        impl/Server.cpp
        impl/Socket.cpp
        impl/TableState.cpp
        impl/Util.cpp
)

# Find dependencies
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

# Build static library
add_library(botball-gamestate STATIC ${SOURCES})
target_link_libraries(botball-gamestate PRIVATE spdlog::spdlog fmt::fmt)
target_include_directories(botball-gamestate PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Install library and headers
install(TARGETS botball-gamestate
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include/botball
        FILES_MATCHING PATTERN "*.h"
)