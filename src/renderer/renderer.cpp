#include "renderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/io.hpp>

#include "u_blocks.h"
#include "assets/materials.h"
#include "backend/glfw_backend.h"
#include "deprecated/texture2d.h"
#include "renderer/vertex.h"
#include "world/transform.h"
#include "world/world.h"
#include "world/camera/camera.h"

// Cur implementation:
// Single buffer for all models

namespace Renderer {
    std::unordered_map<std::string, ShaderProgram> g_availablePrograms;
    ShaderProgram* g_activeProgram = nullptr;
    unsigned int g_meshVAO;
    unsigned int g_meshVBO;
    unsigned int g_meshEBO;
    
    unsigned int g_pointsVAO;
    unsigned int g_pointsVBO;

    unsigned int g_material_map_array;

    unsigned int g_lightingUBO;
    unsigned int g_materialUBO;


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


    void init(const std::string& program_id) {
        use_program(program_id);

        GLFW::add_frame_buffer_size_callback(_frame_buffer_size_callback);
        
        _setup_points();
        
        _setup_models();
        _load_models();

        _init_textures();
        _load_textures();
        
        _init_lights();
        _load_lights();
        
        _init_materials();
        
        glEnable(GL_DEPTH_TEST);
    }

    void begin_draw() {
        glm::vec4 bg_color = World::get_main_camera().get_background_color();
        glClearColor(bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        g_activeProgram->set_mat4("uVP", World::get_main_camera().get_vp_matrix());
        g_activeProgram->set_vec3("uCameraPos", World::get_main_camera().transform.position);
        g_activeProgram->set_vec2("uResolution", glm::vec2(GLFW::get_window_width(), GLFW::get_window_height()));
        
        glPointSize(5.0f);
    }

    void draw_mesh(const Assets::Mesh& mesh, const Transform& transform, const Assets::Material& material) {
        g_activeProgram->set_mat4("uModel", transform.get_matrix());
        _load_material(material);

        std::span<const unsigned int> modelIndices = Assets::get_mesh_indices(mesh);
        size_t indicesOffset = (modelIndices.begin() - Assets::get_all_mesh_indices().begin()) * sizeof(unsigned int);

        glDrawElements(GL_TRIANGLES, modelIndices.size(), GL_UNSIGNED_INT, reinterpret_cast<void *>(indicesOffset));
    }

    void draw_vertices(const std::vector<glm::vec3>& vertices, bool as_points) {
        g_activeProgram->set_mat4("uModel", glm::identity<glm::mat4>());
        
        glBindVertexArray(g_pointsVAO);
        glBindBuffer(GL_ARRAY_BUFFER, g_pointsVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

        GLuint draw_mode = as_points ? GL_POINTS : GL_TRIANGLES;
        glDrawArrays(draw_mode, 0, vertices.size());
    }

    void create_program(const std::string &program_id, const ShaderProgramInfo &program_info) {
        g_availablePrograms.insert_or_assign(program_id, ShaderProgram{program_info});
    }

    void use_program(const std::string &program_id) {
        try {
            ShaderProgram& program = g_availablePrograms.at(program_id);
            program.use();
            g_activeProgram = &program;
        } catch (std::out_of_range&) {
            throw std::runtime_error("No shader program of id: " + program_id);
        }
    }
}
