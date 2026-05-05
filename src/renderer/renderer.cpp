#include "renderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <bit>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/io.hpp>
#include <imgui.h>
#include <windows.h>

#include "u_blocks.h"
#include "assets/materials.h"
#include "assets/splat.h"
#include "backend/glfw_backend.h"
#include "deprecated/texture2d.h"
#include "glm/gtx/string_cast.hpp"
#include "renderer/vertex.h"
#include "utility/config/config.h"
#include "world/transform.h"
#include "world/world.h"
#include "world/camera/camera.h"

// Cur implementation:
// Single buffer for all models

// Force dedicated GPU (system-dependent, can comment out if integrated supports all extensions; mine doesn't)
extern "C" {
    // For NVIDIA
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    // For AMD
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace Renderer {
    const unsigned int SPLAT_KEY_BLOCK_SIZE = 256;
    const unsigned int SPLAT_SORT_DIGIT_SIZE = 8;
    const unsigned int SPLAT_SORT_NUM_DIGITS = 1 << SPLAT_SORT_DIGIT_SIZE;
    
    const unsigned int SPLAT_SCAN_BLOCK_SIZE = 512;
    const unsigned int SPLAT_TOP_SCAN_BLOCK_SIZE = 2048;
    
    std::unordered_map<std::string, ShaderProgram> g_available_programs;
    ShaderProgram* g_active_program = nullptr;
    unsigned int g_meshVAO;
    unsigned int g_meshVBO;
    unsigned int g_meshEBO;
    
    unsigned int g_pointsVAO;
    unsigned int g_pointsVBO;

    unsigned int g_material_map_array;

    unsigned int g_lightingUBO;
    unsigned int g_materialUBO;
    
    // Splat rendering
    unsigned int g_splatSSBO;
    unsigned int g_screen_splatSSBO;
    unsigned int g_splat_INDB;
    
    unsigned int g_num_splats;

    // Splat sorting
    unsigned int g_keysA_SSBO;
    unsigned int g_keysB_SSBO;
    unsigned int g_valuesA_SSBO;
    unsigned int g_valuesB_SSBO;
    unsigned int g_histogram_SSBO;
    
    unsigned int g_key_buffer_size;
    unsigned int g_num_key_blocks;
    unsigned int g_histogram_size;


    unsigned int g_in_keysSSBO;
    unsigned int g_out_keysSSBO;
    unsigned int g_histogramSSBO;

    unsigned int g_emptyVAO;


    unsigned int _texture_format(const int n_channels) {
        switch (n_channels) {
            case 1:
                return GL_RED;
            case 2:
                return GL_RG;
            case 3:
                return GL_RGB;
            default:
                return GL_RGBA;
        }
    }


    void _frame_buffer_size_callback(GLFWwindow* _, const int width, const int height) {
        glViewport(0, 0, width, height);
    }

    /** Initializes model data VAO, VBO and EBO */
    void _setup_models() {
        glGenVertexArrays(1, &g_meshVAO);
        glBindVertexArray(g_meshVAO);

        glGenBuffers(1, &g_meshVBO);
        glBindBuffer(GL_ARRAY_BUFFER, g_meshVBO);

        glGenBuffers(1, &g_meshEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_meshEBO);

        for (const VertexAttributeDesc& desc : Vertex::layout) {
            glVertexAttribPointer(desc.index, desc.size, desc.type, GL_FALSE, sizeof(Vertex), (void*)desc.offset);
            glEnableVertexAttribArray(desc.index);
        }
    }
    
    /** Initializes point data VAO & VBO */
    void _setup_points() {
        glGenVertexArrays(1, &g_pointsVAO);
        glBindVertexArray(g_pointsVAO);
        
        glGenBuffers(1, &g_pointsVBO);
        glBindBuffer(GL_ARRAY_BUFFER, g_pointsVBO);
        
        // Position is only attribute at index 0
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);
    }

    /* Load model data into currently bound VBO and EBO */
    void _load_models() {
        std::span<const Vertex> modelVertices = Assets::get_all_mesh_vertices();
        std::span<const unsigned int> modelIndices = Assets::get_all_mesh_indices();
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * modelVertices.size(), modelVertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * modelIndices.size(), modelIndices.data(), GL_STATIC_DRAW);
    }

    /* Initializes and binds texture arrays */
    void _init_textures() {
        // Material maps
        glGenTextures(1, &g_material_map_array);
        glActiveTexture(GL_TEXTURE0); // Material maps in unit 0 TODO: don't hardcode this
        glBindTexture(GL_TEXTURE_2D_ARRAY, g_material_map_array);
    }

    /* Load texture data */
    void _load_textures() {
        glActiveTexture(GL_TEXTURE0); // Material maps in unit 0 TODO: don't hardcode this

        std::span<const Assets::Texture2D> material_maps = Assets::get_all_material_maps();
        if (material_maps.size() <= 0) return;

        unsigned int width = material_maps[0].width;
        unsigned int height = material_maps[0].height;
        unsigned int n_channels = material_maps[0].n_channels;
        unsigned int format = _texture_format(n_channels);
        
        
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height,
            material_maps.size(), 0, format, GL_UNSIGNED_BYTE, nullptr);
        for (size_t mat = 0; mat < material_maps.size(); ++mat) {
            const std::byte* data = Assets::get_texture_data(material_maps[mat]).data();
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, mat, width, height, 1,
                _texture_format(material_maps[mat].n_channels), GL_UNSIGNED_BYTE, data);
        }

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);  // Set mirrored wrapping
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // When texel:pixel ratio is high (above 1), texture is downscaled/minified, and we decide to use nearest filtering
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   // When texel:pixel ratio is low (below 1), texture is upscaled/magnified, and we decide to use bilinear filtering
    }

    /* Initializes lighting buffers */
    void _init_lights() {
        glGenBuffers(1, &g_lightingUBO);
    }

    /* Load lighting data */
    void _load_lights() {
        UBLighting lighting_block;
        std::span<const World::Light> lights = World::get_all_lights();
        for (int i = 0; i < lights.size(); ++i) {
            lighting_block.lights[i] = UBLight(lights[i]);
        }
        lighting_block.num_lights = lights.size();


        glBindBuffer(GL_UNIFORM_BUFFER, g_lightingUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UBLighting), &lighting_block, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_lightingUBO);  // bind the UBO to UBO binding 0, referred to in shader with `binding = 0`
    }
    
    /* Initializes material buffer */
    void _init_materials() {
        glGenBuffers(1, &g_materialUBO);
    }
    
    void _load_material(const Assets::Material& material) {
        UBMaterial ub_material{material};
        
        glBindBuffer(GL_UNIFORM_BUFFER, g_materialUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UBMaterial), &ub_material, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, g_materialUBO);  // Bind material UBO to binding 1
    }

    void _setup_splats()
    {
        glGenBuffers(1, &g_splatSSBO);
        glGenBuffers(1, &g_screen_splatSSBO);
        glGenBuffers(1, &g_splat_INDB);
        glGenBuffers(1, &g_keysA_SSBO);
        glGenBuffers(1, &g_keysB_SSBO);
        glGenBuffers(1, &g_valuesA_SSBO);
        glGenBuffers(1, &g_valuesB_SSBO);
        glGenBuffers(1, &g_histogram_SSBO);
        glGenVertexArrays(1, &g_emptyVAO);
    }
    
    void _load_splats() {
        // Load splat data into splat buffer, binding = 1
        std::span<const Assets::Splat> splats = Assets::get_splats_sorted(World::get_main_camera().get_view_matrix());
        //std::span<const Assets::Splat> splats = Assets::get_splats();
        std::cout << "Num splats: " << splats.size() << std::endl;
        std::vector<SSBSplat> ssb_splats{};
        ssb_splats.reserve(splats.size());
        for (const Assets::Splat& splat : splats) {
            ssb_splats.push_back(SSBSplat{splat});
        }
        g_num_splats = ssb_splats.size();
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_splatSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ssb_splats.size() * sizeof(SSBSplat), ssb_splats.data(), GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_splatSSBO);
        
        // Init screen splat buffer, binding = 0
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_screen_splatSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ssb_splats.size() * sizeof(SSBScreenSplat), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_screen_splatSSBO);

        // Init indirect draw arrays command buffer
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_splat_INDB);
        SSBDrawArraysIndirectCommand cmd = {4, 0, 0, 0};
        glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(SSBDrawArraysIndirectCommand), &cmd, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_splat_INDB);
        
        
        // Init buffers for splat sorting
        g_key_buffer_size = std::bit_ceil(g_num_splats);  // Ensure keys buffer size is power of 2 to simplify radix sort
        g_num_key_blocks = g_key_buffer_size / SPLAT_KEY_BLOCK_SIZE;
        g_histogram_size = g_num_key_blocks * SPLAT_SORT_NUM_DIGITS;
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_keysA_SSBO);  // Input keys, initialize all to max value
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint64_t) * g_key_buffer_size, nullptr, GL_DYNAMIC_DRAW);
        GLubyte fill = 0xFF;
        glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R8I, GL_RED_INTEGER, GL_UNSIGNED_BYTE, &fill);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, g_keysA_SSBO);
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_keysB_SSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint64_t) * g_key_buffer_size, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, g_keysB_SSBO);
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_valuesA_SSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::uint) * g_key_buffer_size, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, g_valuesA_SSBO);
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_valuesB_SSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::uint) * g_key_buffer_size, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, g_valuesB_SSBO);
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_histogram_SSBO);  // Digit histogram
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::uint) * g_histogram_size, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, g_valuesB_SSBO);
    }
    
    void _init_programs() {
        // TODO: glslangValidator for early compilation check
        std::string comp_path = Config::get_value(Config::ConfigGroup::Shaders, "shaders", "comp");
        std::string vert_path = Config::get_value(Config::ConfigGroup::Shaders, "shaders", "vert");
        std::string frag_path = Config::get_value(Config::ConfigGroup::Shaders, "shaders", "frag");
       

        ShaderProgram proj_program = ShaderProgram{
                    {ShaderType::Compute}, {comp_path}
        };
        add_program("project_splats", proj_program);
        ShaderProgram render_program = ShaderProgram{
                    {ShaderType::Vertex, ShaderType::Fragment},
                    {vert_path, frag_path}
        };
        add_program("render_splats", render_program);
        
        std::string points_vert_path = Config::get_value(Config::ConfigGroup::Shaders, "shaders", "points_vert");
        std::string points_frag_path = Config::get_value(Config::ConfigGroup::Shaders, "shaders", "points_frag");
        ShaderProgram points_program = ShaderProgram{
                {ShaderType::Vertex, ShaderType::Fragment},
                {points_vert_path, points_frag_path}
        };
        add_program("points", points_program);

        std::string upsweep = Config::get_value(Config::ConfigGroup::Shaders, "radix_sort", "upsweep");
        add_program("upsweep", ShaderProgram{{ShaderType::Compute}, {upsweep}});
        
        std::string downsweep = Config::get_value(Config::ConfigGroup::Shaders, "radix_sort", "downsweep");
        add_program("downsweep", ShaderProgram{{ShaderType::Compute}, {downsweep}});
        
        std::string top = Config::get_value(Config::ConfigGroup::Shaders, "radix_sort", "top");
        add_program("top", ShaderProgram{{ShaderType::Compute}, {top}});
        
        std::string build_histogram = Config::get_value(Config::ConfigGroup::Shaders, "radix_sort", "build_histogram");
        add_program("build_histogram", ShaderProgram{{ShaderType::Compute}, {build_histogram}});
        
        std::string local_sort = Config::get_value(Config::ConfigGroup::Shaders, "radix_sort", "local_sort");
        add_program("local_sort", ShaderProgram{{ShaderType::Compute}, {local_sort}});
    }

    void init() {
        _init_programs();
        
        GLFW::add_frame_buffer_size_callback(_frame_buffer_size_callback);
        
        _setup_points();
        _setup_splats();
        _load_splats();
        
        // _setup_models();
        // _load_models();
        //
        // _init_textures();
        // _load_textures();
        //
        // _init_lights();
        // _load_lights();
        //
        // _init_materials();
        
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void begin_draw() {
        glm::vec4 bg_color = World::get_main_camera().get_background_color();
        glClearColor(bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //g_activeProgram->set_mat4("uVP", World::get_main_camera().get_vp_matrix());
        // g_activeProgram->set_mat4("uView", World::get_main_camera().get_view_matrix());
        // g_activeProgram->set_mat4("uProj", World::get_main_camera().get_proj_matrix());
        // g_activeProgram->set_vec3("uCameraPos", World::get_main_camera().transform.position);
        //g_activeProgram->set_vec2("uResolution", glm::vec2(GLFW::get_window_width(), GLFW::get_window_height()));
        
        glPointSize(5.0f);
    }

    void draw_mesh(const Assets::Mesh& mesh, const Transform& transform, const Assets::Material& material) {
        g_active_program->set_mat4("uModel", transform.get_matrix());
        _load_material(material);

        std::span<const unsigned int> modelIndices = Assets::get_mesh_indices(mesh);
        size_t indicesOffset = (modelIndices.begin() - Assets::get_all_mesh_indices().begin()) * sizeof(unsigned int);

        glDrawElements(GL_TRIANGLES, modelIndices.size(), GL_UNSIGNED_INT, reinterpret_cast<void *>(indicesOffset));
    }

    void draw_vertices(const std::vector<glm::vec3>& vertices, bool as_points) {
        if (as_points) use_program("points");

        // std::cout << "Projection" << World::get_main_camera().get_proj_matrix() << std::endl;
        // std::cout << World::get_main_camera().get_vp_matrix() << std::endl;
        // std::cout << "First point:" << vertices[0] << std::endl;
        // std::cout << "Projected:" << World::get_main_camera().get_vp_matrix() * glm::vec4(vertices[0], 1.0) << std::endl;

        g_active_program->set_mat4("uModel", glm::identity<glm::mat4>());
        g_active_program->set_mat4("uVP", World::get_main_camera().get_vp_matrix());
        
        glBindVertexArray(g_pointsVAO);
        glBindBuffer(GL_ARRAY_BUFFER, g_pointsVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

        GLuint draw_mode = as_points ? GL_POINTS : GL_TRIANGLES;
        glDrawArrays(draw_mode, 0, vertices.size());
    }
    
    /** Performs Blelloch prefix scan over the digit histogram buffer */
    void _scan_histogram() {
        unsigned int stride = 1;
        
        // Upsweeps until current level fits in single up/down
        use_program("upsweep");
        while (g_histogram_size / stride > SPLAT_TOP_SCAN_BLOCK_SIZE) {
            g_active_program->set_uint("uBaseStride", stride);
            glDispatchCompute(g_histogram_size / (stride * SPLAT_SCAN_BLOCK_SIZE), 1, 1);
            stride *= SPLAT_SCAN_BLOCK_SIZE;
        }
        
        // Top-level upsweep & downsweep
        use_program("top");
        g_active_program->set_uint("uBaseStride", stride);
        glDispatchCompute(1, 1, 1);
        
        stride /= SPLAT_SCAN_BLOCK_SIZE;
        
        // Downsweeps until scan complete
        use_program("downsweep");
        while (stride > 0) {
            g_active_program->set_uint("uBaseStride", stride);
            glDispatchCompute(g_histogram_size / (stride * SPLAT_SCAN_BLOCK_SIZE), 1, 1);
            stride /= SPLAT_SCAN_BLOCK_SIZE;
        }
    }
    
    /** Radix sorts all splats using the GPU, must be invoked before rendering pass */
    void _sort_splats() {
        for (int i = 0; i < SPLAT_SORT_NUM_DIGITS; ++i) {
            // Swap key/value buffers (3/5 in, 4/6 out)
            bool A_in = i % 2 == 0;
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, A_in ? g_keysA_SSBO : g_keysB_SSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, A_in ? g_keysB_SSBO : g_keysA_SSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, A_in ? g_valuesA_SSBO : g_valuesB_SSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, A_in ? g_valuesB_SSBO : g_valuesA_SSBO);
            
            GLubyte fill = 0x00; // Reset histogram
            glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R8I, GL_RED_INTEGER, GL_UNSIGNED_BYTE, &fill);
            
            // Build digit histogram
            use_program("build_histogram");
            g_active_program->set_uint("uNumBlocks", g_num_key_blocks);
            g_active_program->set_uint("uDigitId", i);
            glDispatchCompute(g_num_key_blocks, 1, 1);
            
            // Blelloch prefix scan
            _scan_histogram();
            
            use_program("local_sort");
            g_active_program->set_uint("uNumBlocks", g_num_key_blocks);
            g_active_program->set_uint("uDigitId", i);
            glDispatchCompute(g_num_key_blocks, 1, 1);
        }
    }

    void draw_splats() {
        const int WG_SIZE = 256;

        // TESTING:
        // ImGui::Begin("Splats");
        // glm::vec4 splat_screenspace = World::get_main_camera().get_proj_matrix() * World::get_main_camera().get_view_matrix() * glm::vec4(Assets::get_splats()[0].position, 1.0);
        // glm::vec2 splat_ndc = glm::vec2(splat_screenspace.x, splat_screenspace.y) / splat_screenspace.w;
        // ImGui::Text(("Center of splat 0: " + glm::to_string(splat_ndc)).c_str());
        // ImGui::End();

        const Camera& main_camera = World::get_main_camera();

        use_program("project_splats");
        g_active_program->set_mat4("uProjection", main_camera.get_proj_matrix());
        g_active_program->set_mat4("uView", main_camera.get_view_matrix());
        g_active_program->set_vec3("uCameraPos", main_camera.transform.position);
        g_active_program->set_uint("uNumSplats", g_num_splats);
        g_active_program->set_vec2("uTanFov", main_camera.tan_fov());

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_splat_INDB);
        glm::uint zero = 0;
        glBufferSubData(GL_DRAW_INDIRECT_BUFFER, offsetof(SSBDrawArraysIndirectCommand, instance_count), sizeof(glm::uint), &zero);
        glDispatchCompute((g_num_splats + WG_SIZE - 1) / WG_SIZE, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
        
        // Radix sort
        _sort_splats();

        use_program("render_splats");
        g_active_program->set_mat4("uView", main_camera.get_view_matrix());
        g_active_program->set_mat4("uProj", main_camera.get_proj_matrix());
        g_active_program->set_vec3("uCameraPos", main_camera.transform.position);
        g_active_program->set_vec3("uBackgroundColor", main_camera.get_background_color());

        glBindVertexArray(g_emptyVAO);
        glDrawArraysIndirect(GL_TRIANGLE_STRIP, (void*)0);
    }

    void add_program(const std::string& program_id, const ShaderProgram& program) {
        g_available_programs.insert_or_assign(program_id, program);
    }

    void use_program(const std::string& program_id) {
        try {
            ShaderProgram& program = g_available_programs.at(program_id);
            program.use();
            g_active_program = &program;
        } catch (std::out_of_range&) {
            throw std::runtime_error("No shader program of id: " + program_id);
        }
    }
}
