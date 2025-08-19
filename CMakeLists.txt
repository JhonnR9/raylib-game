cmake_minimum_required(VERSION 3.31)
project(raylib_game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")

add_compile_definitions(BUILD_ATLAS_MODE=0)

add_executable(raylib_game
        src/main.cpp
        src/engine/app.cpp
        src/utils/texture_packer.cpp
        src/game/factories/entities_factory.cpp
        src/game/scenes/my_scene.cpp
        src/game/systems/player_input_system.cpp
        src/engine/systems/camera_system.cpp
        src/engine/systems/collision_detection_system.cpp
        src/engine/systems/move_system.cpp
        src/engine/systems/overlap_correction_system.cpp
        src/engine/systems/shape_render_system.cpp
        src/engine/systems/sprite_renderer_system.cpp
)

target_compile_definitions(raylib_game PRIVATE RESOURCE_PATH="resources")

target_include_directories(raylib_game PRIVATE
        "${CMAKE_SOURCE_DIR}/src"
)

# raylib library for mingw
add_library(raylib SHARED IMPORTED)
set_target_properties(raylib PROPERTIES
        IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/external/raylib/lib/libraylibdll.a"
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/external/raylib/lib/raylib.dll"
)
target_include_directories(raylib INTERFACE
        "${CMAKE_SOURCE_DIR}/external/raylib/include"
)

# EnTT (header-only)
target_include_directories(raylib_game PRIVATE
        "${CMAKE_SOURCE_DIR}/external/entt-3.15.0/single_include"
)

# nlohmann (header-only)
target_include_directories(raylib_game PRIVATE
        "${CMAKE_SOURCE_DIR}/external/nlohmann/include"
)

# stb_image (header-only)
target_include_directories(raylib_game PRIVATE
        "${CMAKE_SOURCE_DIR}/external/stb_image/include"
)

target_link_libraries(raylib_game PRIVATE raylib)

add_custom_command(TARGET raylib_game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${RESOURCE_DIR}" "$<TARGET_FILE_DIR:raylib_game>/resources"
)
