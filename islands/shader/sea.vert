#version 330 core

uniform mat4 M;
uniform mat4 VP;
uniform float time;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

out vec2 uv;
out vec3 normal;

void main() {
    vec4 worldPos = M * vec4(pos, 1);
    worldPos.z += 0.3 * sin(time + worldPos.x - worldPos.y) - 0.3;
    gl_Position = VP * worldPos;
    uv = in_uv;
    normal = in_normal;
}
