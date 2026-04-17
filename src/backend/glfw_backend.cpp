#include "glfw_backend.h"

#include <iostream>
#include <exception>

#include "glfw_input.h"

namespace GLFW {
    GLFWwindow* g_window;
    std::vector<FrameBufferSizeCallback> g_frameBufferSizeCallbacks;
    
    int g_window_width = 400;
    int g_window_height = 400;

    void _frame_buffer_size_callback(GLFWwindow* window, const int width, const int height) {
        g_window_width = width;
        g_window_height = height;
        for (auto callback : g_frameBufferSizeCallbacks) {
            callback(window, width, height);
        }
    }

    
    void init() {
#ifdef __linux__
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        g_window = glfwCreateWindow(g_window_width, g_window_height, "Water", nullptr, nullptr);
        if (!g_window) {
            glfwTerminate();
            throw glfw_runtime_error("Failed to create GLFW window!");
        }
        glfwMakeContextCurrent(g_window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw glfw_runtime_error("Failed to initialize GLAD!");
        }

        glfwSetFramebufferSizeCallback(g_window, _frame_buffer_size_callback);

        Input::init(g_window);
    }

    void end_frame() {
        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }

    void destroy() {
        glfwTerminate();
    }

    bool window_should_close() {
        return glfwWindowShouldClose(g_window);
    }

    void set_window_should_close(bool should_close) {
        glfwSetWindowShouldClose(g_window, should_close);
    }

    GLFWwindow* get_window_ptr() {
        return g_window;
    }

    void set_window_width(const int width) {
        g_window_width = width;
        glfwSetWindowSize(g_window, g_window_width, g_window_height);
    }
    int get_window_width() {
        return g_window_width;
    }

    void set_window_height(const int height) {
        g_window_height = height;
        glfwSetWindowSize(g_window, g_window_width, g_window_height);
    }
    int get_window_height() {
        return g_window_height;
    }

    float get_aspect_ratio() {
        return g_window_width / g_window_height;
    }

    void add_frame_buffer_size_callback(const FrameBufferSizeCallback& callback) {
        g_frameBufferSizeCallbacks.push_back(callback);
    }

    double get_time() {
        return glfwGetTime();
    }
}
