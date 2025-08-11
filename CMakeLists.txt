cmake_minimum_required(VERSION 3.31)
project(raylib_game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")
add_definitions(-DRESOURCE_PATH="${RESOURCE_DIR}")

include(FetchContent)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        5.5
)

FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG        v3.15.0
)

FetchContent_MakeAvailable(entt)

add_executable(raylib_game src/main.cpp)

target_include_directories(raylib_game PRIVATE ${entt_SOURCE_DIR}/single_include)

target_link_libraries(raylib_game PRIVATE raylib)


if(UNIX AND NOT APPLE)
    target_link_libraries(raylib_game PRIVATE m pthread dl rt)
endif()
