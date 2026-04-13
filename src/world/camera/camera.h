#ifndef WATERENGINE_CAMERA_H
#define WATERENGINE_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "world/transform.h"
#include "world/frame_context.h"

// TODO: refactor derived "FlyCamera" class if we want base class interface for cameras instead of only one type

/* A Camera, representing where and at what perspective the scene is rendered from */
struct Camera {
    Camera();
    explicit Camera(const Transform& transform);
    
    Transform transform;

    /* Generate camera's view matrix */
    [[nodiscard]] glm::mat4 get_view_matrix() const;
    /* Generate camera's projection matrix */
    [[nodiscard]] glm::mat4 get_proj_matrix() const;
    /* Generate camera's projection * view matrix */
    [[nodiscard]] glm::mat4 get_vp_matrix() const;

    /* Set then camera's background color */
    void set_background_color(const glm::vec4& color);
    /* Retrieve the camera's background color */
    [[nodiscard]] glm::vec4 get_background_color() const;

    /* Set the camera's vertical FOV, in radians */
    void set_fovy(float fovy);
    /* Retrieve the camera's vertical FOV, in radians */
    [[nodiscard]] float get_fovy() const;
private:
    glm::vec4 m_background_color = glm::vec4(0.2f, 0.25f, 0.3f, 1.0f);
    /* Vertical FOV, in radians */
    float m_fovy = glm::radians(90.0f);
    float m_near = 0.1f;
    float m_far = 1000.0f;

    float m_prevCursorX = -1.0f;
    float m_prevCursorY = -1.0f;
    float m_sensitivity = 0.01f;
    glm::vec3 m_rot;

    float m_speed = 3.0f;

    void m_cursor_pos_callback(double xpos, double ypos);
    void m_update(const World::FrameContext& frame_context);
    void m_update_movement(float delta_time);
};

// need to detect whether user is pressing certain keys at some time
// need to do it per frame

#endif //WATERENGINE_CAMERA_H