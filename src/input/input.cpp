#include "input.h"

#include <iostream>
#include <ostream>

#include "imgui.h"
#include "backend/glfw_enums.h"
#include "backend/glfw_input.h"

namespace Input {
    CursorMode g_cursorMode = CursorMode::GAME;
    
    void append_cursor_pos_callback(CursorPosCallback callback) {
        GLFW::Input::append_cursor_pos_callback(callback);
    }
    
    void append_key_callback(KeyCallback callback) {
        GLFW::Input::append_key_callback([callback](int key, int scancode, int action, int mods) {
            callback(from_glfw_key.at(key), from_glfw_action.at(action));
        });
    }

    void append_mouse_button_callback(const MouseButtonCallback &callback) {
        GLFW::Input::append_mouse_button_callback([callback](int button, int action, int mods) {
            callback(from_glfw_mouse_button.at(button), from_glfw_action.at(action));
        });
    }

    bool is_pressed(Key key) {
        return GLFW::Input::is_pressed(to_glfw_key.at(key));
    }
    
    void _set_cursor_game() {
        GLFW::Input::set_focus_cursor(true);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
    }
    void _set_cursor_gui() {
        GLFW::Input::set_focus_cursor(false);
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    }
    
    void set_cursor_mode(const CursorMode& cursor_mode) {
        switch (cursor_mode) {
            case CursorMode::GAME:
                _set_cursor_game();
                return;
            case CursorMode::GUI:
                _set_cursor_gui();
                return;
        }
    }
}
