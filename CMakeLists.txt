cmake_minimum_required(VERSION 4.0.2)
project(waterengine)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(deps/glfw)
add_subdirectory(deps/simpleini)

file(GLOB_RECURSE LOCAL_SRC CONFIGURE_DEPENDS
        src/assets/meshes.cpp
        src/assets/materials.cpp
        src/assets/texture2ds.cpp

        src/backend/glfw_backend.cpp
        src/backend/glfw_input.cpp
        src/backend/glfw_enums.h

        src/input/input.cpp
        src/input/enums.h

        src/renderer/renderer.cpp
        src/renderer/shader_program.cpp
        src/deprecated/texture2d.cpp

        src/world/camera/camera.cpp
        src/world/transform.cpp
        src/world/update_registry.cpp
        src/world/world.cpp
        src/world/lighting.cpp

        src/main.cpp
)
file(GLOB IMGUI_SRC CONFIGURE_DEPENDS
        "deps/imgui/imgui.cpp"
        "deps/imgui/imgui_demo.cpp"
        "deps/imgui/imgui_draw.cpp"
        "deps/imgui/imgui_tables.cpp"
        "deps/imgui/imgui_widgets.cpp"

        "deps/imgui/backends/imgui_impl_glfw.cpp"
        "deps/imgui/backends/imgui_impl_opengl3.cpp"
        "deps/imgui/misc/cpp/imgui_stdlib.cpp"
)

add_executable(waterengine
        deps/glad/src/glad.c

        ${IMGUI_SRC}
        ${LOCAL_SRC}
        src/assets/material_flags.h
        src/utility/bitmask.h
        src/utility/tangent_space.h
        src/utility/tangent_space.cpp
        src/utility/config/config.h
        src/utility/config/config.cpp
        deps/stb/stb_include.h
)

target_include_directories(waterengine PUBLIC 
        src 
        deps/glfw/include 
        deps/glad/include 
        deps/glm 
        deps/stb 
        deps/spdlog/include 
        deps/imgui 
        deps/simpleini
        deps/nlohmann/single_include
        deps/happly
)
target_link_libraries(waterengine glfw ${GLFW_LIBRARIES} SimpleIni::SimpleIni)
set_target_properties(waterengine PROPERTIES DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
