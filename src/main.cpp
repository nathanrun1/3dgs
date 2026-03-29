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

    // Assets
    std::vector vertices(std::begin(cube_vertices), std::end(cube_vertices));
    std::vector indices(std::begin(cube_indices), std::end(cube_indices));
    calculate_tangents(vertices, indices);
    Assets::Mesh cube = Assets::create_mesh(vertices, indices);

    // Assets::Texture2D container = Assets::create_texture2d("res/textures/container.jpg");
    // Assets::MaterialInfo material_info{};
    // material_info.albedo_map = container;
    // material_info.roughness_scale = 0.5;
    // material_info.metallic_scale = 0.0;
    // Assets::Material container_material = Assets::create_material(material_info);

    Assets::MaterialInfo material_info_light{};
    material_info_light.albedo_scale = glm::vec3(1.0);
    material_info_light.roughness_scale = 1.0;
    material_info_light.metallic_scale = 0.0;
    material_info_light.flags |= Assets::MaterialFlag::Unlit;
    Assets::Material light_material = Assets::create_material(material_info_light);
    
    Assets::MaterialInfo material_info_paving_stones{};
    material_info_paving_stones.albedo_map = Assets::create_texture2d("res/textures/paving_stones/albedo.jpg");
    material_info_paving_stones.roughness_map = Assets::create_texture2d("res/textures/paving_stones/roughness.jpg");
    material_info_paving_stones.normal_map = Assets::create_texture2d("res/textures/paving_stones/normal_gl.jpg");
    material_info_paving_stones.displacement_map = Assets::create_texture2d("res/textures/paving_stones/displacement.jpg");
    material_info_paving_stones.displacement_scale = 5.0f;
    Assets::Material stones_material = Assets::create_material(material_info_paving_stones);

    // Lights
    glm::vec3 light_pos = glm::vec3{3.0f, 5.0f, 0.0f};
    World::add_light(World::Light{
        light_pos,
        World::LightType::Point,
        {1.0f, 1.0f, 1.0f},
        1.0f
    });
    World::add_light(World::Light{
        {0.0f, 0.0f, 0.0f},
        World::LightType::Ambient,
        {1.0f, 1.0f, 1.0f},
        0.1f
    });

    // Shaders
    ShaderProgramInfo sp_info{
        Config::get_value(Config::ConfigGroup::Shaders, "shaders", "vert"),
        Config::get_value(Config::ConfigGroup::Shaders, "shaders", "frag")
    };
    Renderer::create_program("default", sp_info);

    World::init();
    Renderer::init("default");
    
    std::cout << "check" << std::endl;
    
    while (!GLFW::window_should_close()) {
        Renderer::begin_draw();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        World::UpdateRegistry::run_all_callbacks();

        // // Draw containers
        // for (glm::vec3& pos : cubePositions) {
        //     Transform transform;
        //     transform.position = pos;
        //     //transform.set_euler_angles(0.0f, glfwGetTime() * glm::radians(90.0f), 0.0f);
        //     transform.scale = {2.0f, 1.0f, 2.0f};
        //     Renderer::draw_mesh(cube, transform, stones_material);
        // }
        //
        // // Draw light object
        // Transform light_transform;
        // light_transform.position = light_pos;
        // light_transform.scale = glm::vec3(0.5f);
        // Renderer::draw_mesh(cube, light_transform, light_material);
        
        Renderer::draw_points(points);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        GLFW::end_frame();
    }
    GLFW::destroy();
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}