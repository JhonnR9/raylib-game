cmake_minimum_required(VERSION 3.31)
project(raylib_game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")

add_compile_definitions(BUILD_ATLAS_MODE=0)

# ====== Opções de Build ======
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the build type: Debug or Release" FORCE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
# ============================

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

FetchContent_Declare(
        nlohmann
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.12.0
)
FetchContent_MakeAvailable(nlohmann)

add_executable(raylib_game
        src/main.cpp
        src/utils/texture_packer.cpp
        src/utils/texture_packer.h
        src/app.cpp
        src/app.h
        src/scenes/scene.h
        src/scenes/my_scene.cpp
        src/scenes/my_scene.h
        src/components/components.h
        src/systems/render_system.cpp
        src/systems/render_system.h
        src/systems/system.h
        src/systems/player_input_system.h
        src/systems/player_input_system.cpp
        src/systems/move_system.cpp
        src/systems/move_system.h
        src/systems/collision_detection_system.cpp
        src/systems/collision_detection_system.h
        src/systems/overlap_correction_system.cpp
        src/systems/overlap_correction_system.h
        src/utils/global_utils.h)

target_include_directories(raylib_game
        PRIVATE
        ${entt_SOURCE_DIR}/src
        ${nlohmann_SOURCE_DIR}/include
        vendor/stb_image
)

target_compile_definitions(raylib_game PRIVATE RESOURCE_PATH="${RESOURCE_DIR}")

target_link_libraries(raylib_game PRIVATE raylib)

if(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(raylib_game PRIVATE m pthread dl rt OpenGL::GL)
endif()

if(MINGW)
    target_link_options(raylib_game PRIVATE
            "-static-libgcc"
            "-static-libstdc++"
    )
endif()

