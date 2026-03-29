#version 460

layout (location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uVP;

void main() {
    gl_Position = uVP * uModel * vec4(aPos, 1.0);
}
