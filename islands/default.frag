#version 330 core

in vec2 uv;
in vec3 normal;
in vec4 diffuseColor;
out vec3 color;

void main() {
    color = diffuseColor.rgb;
}