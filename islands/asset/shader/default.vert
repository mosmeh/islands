#version 330 core

uniform mat4 MVP;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

out vec2 uv;
out vec3 normal;

void main() {
    gl_Position = MVP * vec4(pos, 1);
    uv = in_uv;
    normal = in_normal;
}
