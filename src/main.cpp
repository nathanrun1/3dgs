#include <fstream>
#include <iostream>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <nlohmann/json.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

#include "renderer/renderer.h"
#include "backend/glfw_backend.h"
#include "backend/glfw_input.h"
#include "input/enums.h"
#include "utility/splat.h"
#include "utility/tangent_space.h"
#include "utility/config/config.h"
#include "world/lighting.h"
#include "world/transform.h"
#include "world/update_registry.h"
#include "world/world.h"

using namespace Input;

Vertex cube_vertices[] = {
    {{-0.5f, -0.5f, -0.5f,}, {0.0f, 0.0f},  { 0.0f,  0.0f, -1.0f}},
    {{ 0.5f, -0.5f, -0.5f,}, {1.0f, 0.0f,}, { 0.0f,  0.0f, -1.0f}},
    {{ 0.5f,  0.5f, -0.5f,}, {1.0f, 1.0f,}, { 0.0f,  0.0f, -1.0f}},

    {{ 0.5f,  0.5f, -0.5f,}, {1.0f, 1.0f,}, { 0.0f,  0.0f, -1.0f}},
    {{-0.5f,  0.5f, -0.5f,}, {0.0f, 1.0f,}, { 0.0f,  0.0f, -1.0f}},
    {{-0.5f, -0.5f, -0.5f,}, {0.0f, 0.0f,}, { 0.0f,  0.0f, -1.0f}},

    {{-0.5f, -0.5f,  0.5f,}, {0.0f, 0.0f,}, { 0.0f,  0.0f,  1.0f}},
    {{ 0.5f, -0.5f,  0.5f,}, {1.0f, 0.0f,}, { 0.0f,  0.0f,  1.0f}},
    {{ 0.5f,  0.5f,  0.5f,}, {1.0f, 1.0f,}, { 0.0f,  0.0f,  1.0f}},

    {{ 0.5f,  0.5f,  0.5f,}, {1.0f, 1.0f,}, { 0.0f,  0.0f,  1.0f}},
    {{-0.5f,  0.5f,  0.5f,}, {0.0f, 1.0f,}, { 0.0f,  0.0f,  1.0f}},
    {{-0.5f, -0.5f,  0.5f,}, {0.0f, 0.0f,}, { 0.0f,  0.0f,  1.0f}},

    {{-0.5f,  0.5f,  0.5f,}, {1.0f, 0.0f,}, {-1.0f,  0.0f,  0.0f}},
    {{-0.5f,  0.5f, -0.5f,}, {1.0f, 1.0f,}, {-1.0f,  0.0f,  0.0f}},
    {{-0.5f, -0.5f, -0.5f,}, {0.0f, 1.0f,}, {-1.0f,  0.0f,  0.0f}},

    {{-0.5f, -0.5f, -0.5f,}, {0.0f, 1.0f,}, {-1.0f,  0.0f,  0.0f}},
    {{-0.5f, -0.5f,  0.5f,}, {0.0f, 0.0f,}, {-1.0f,  0.0f,  0.0f}},
    {{-0.5f,  0.5f,  0.5f,}, {1.0f, 0.0f,}, {-1.0f,  0.0f,  0.0f}},

    {{ 0.5f,  0.5f,  0.5f,}, {1.0f, 0.0f,}, { 1.0f,  0.0f,  0.0f}},
    {{ 0.5f,  0.5f, -0.5f,}, {1.0f, 1.0f,}, { 1.0f,  0.0f,  0.0f}},
    {{ 0.5f, -0.5f, -0.5f,}, {0.0f, 1.0f,}, { 1.0f,  0.0f,  0.0f}},

    {{ 0.5f, -0.5f, -0.5f,}, {0.0f, 1.0f,}, { 1.0f,  0.0f,  0.0f}},
    {{ 0.5f, -0.5f,  0.5f,}, {0.0f, 0.0f,}, { 1.0f,  0.0f,  0.0f}},
    {{ 0.5f,  0.5f,  0.5f,}, {1.0f, 0.0f,}, { 1.0f,  0.0f,  0.0f}},

    {{-0.5f, -0.5f, -0.5f,}, {0.0f, 1.0f,}, { 0.0f, -1.0f,  0.0f}},
    {{ 0.5f, -0.5f, -0.5f,}, {1.0f, 1.0f,}, { 0.0f, -1.0f,  0.0f}},
    {{ 0.5f, -0.5f,  0.5f,}, {1.0f, 0.0f,}, { 0.0f, -1.0f,  0.0f}},

    {{ 0.5f, -0.5f,  0.5f,}, {1.0f, 0.0f,}, { 0.0f, -1.0f,  0.0f}},
    {{-0.5f, -0.5f,  0.5f,}, {0.0f, 0.0f,}, { 0.0f, -1.0f,  0.0f}},
    {{-0.5f, -0.5f, -0.5f,}, {0.0f, 1.0f,}, { 0.0f, -1.0f,  0.0f}},

    {{-0.5f,  0.5f, -0.5f,}, {0.0f, 1.0f,}, { 0.0f,  1.0f,  0.0f}},
    {{ 0.5f,  0.5f, -0.5f,}, {1.0f, 1.0f,}, { 0.0f,  1.0f,  0.0f}},
    {{ 0.5f,  0.5f,  0.5f,}, {1.0f, 0.0f,}, { 0.0f,  1.0f,  0.0f}},

    {{ 0.5f,  0.5f,  0.5f,}, {1.0f, 0.0f,}, { 0.0f,  1.0f,  0.0f}},
    {{-0.5f,  0.5f,  0.5f,}, {0.0f, 0.0f,}, { 0.0f,  1.0f,  0.0f}},
    {{-0.5f,  0.5f, -0.5f},  { 0.0f, 1.0f}, { 0.0f,  1.0f,  0.0f}}
};

unsigned int cube_indices[] = {
     0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35
};

glm::vec3 cube_positions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

std::vector<glm::vec3> fullscreen_quad = {
    glm::vec3(-1.0f,  1.0f, 0.0f),
    glm::vec3(-1.0f, -1.0f, 0.0f),
    glm::vec3( 1.0f, -1.0f, 0.0f),
    glm::vec3(-1.0f,  1.0f, 0.0f),
    glm::vec3( 1.0f, -1.0f, 0.0f),
    glm::vec3( 1.0f,  1.0f, 0.0f),
};

/** Re-initializes the renderer with the given shader program */
void hot_reload(const ShaderProgramInfo& sp_info) {
    std::cout << "Reloading..." << std::endl;
    Renderer::create_program("hot", sp_info);
    Renderer::init("hot");
}

void key_callback(Key key, Action action) {
    if (key == Key::Escape && action == Action::Press) {
        Input::set_cursor_mode(CursorMode::GUI);
    }
    if (key == Key::R && action == Action::Press) {
        hot_reload({
        Config::get_value(Config::ConfigGroup::Shaders, "shaders", "vert"),
        Config::get_value(Config::ConfigGroup::Shaders, "shaders", "frag")
        });
    }
}

void mouse_callback(MouseButton mouse_button, Action action) {
    if (!ImGui::GetIO().WantCaptureMouse && mouse_button == MouseButton::M1 && action == Action::Press)
        Input::set_cursor_mode(CursorMode::GAME);
}

/** Loads points from the given JSON file, assuming the JSON file contains only an array of 3-element float arrays */
std::vector<glm::vec3> load_points(const std::string& json_file) {
    std::ifstream f{json_file};
    nlohmann::json data = nlohmann::json::parse(f);
    
    std::cout << data.size() << std::endl;
    
    std::vector<glm::vec3> points{};
    for (const nlohmann::json& p : data) {
        points.emplace_back((float)p[0], (float)p[1], (float)p[2]);
    }
    
    return points;
}

int main() {
    std::vector<Splat> splats = load_splats("res/data/splat.ply");
    
    
    //std::cout << "Hello?????" << std::endl;
    //std::cout << splats[0].color << std::endl;
    
    std::cout << "check" << std::endl;
    std::vector<glm::vec3> points = load_points("res/data/samples.json");
    
    GLFW::set_window_width(1600);
    GLFW::set_window_height(1200);
    GLFW::init();
    
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(GLFW::get_window_ptr(), true);
    ImGui_ImplOpenGL3_Init();
    
    Input::set_cursor_mode(CursorMode::GAME);
    Input::append_key_callback(key_callback);
    Input::append_mouse_button_callback(mouse_callback);

    // Shaders
    ShaderProgramInfo sp_info{
        Config::get_value(Config::ConfigGroup::Shaders, "shaders", "vert"),
        Config::get_value(Config::ConfigGroup::Shaders, "shaders", "frag")
    };
    Renderer::create_program("default", sp_info);

    World::init();
    Renderer::init("default");
    
    while (!GLFW::window_should_close()) {
        Renderer::begin_draw();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        World::UpdateRegistry::run_all_callbacks();

        Renderer::draw_vertices(fullscreen_quad);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        GLFW::end_frame();
    }
    GLFW::destroy();
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}