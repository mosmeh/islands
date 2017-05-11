#version 330 core

uniform sampler2D lightmap;
in vec2 uv;
in vec3 normal;
in vec4 diffuseColor;
out vec3 color;

void main() {
    color = texture2D(lightmap, uv).rgb;
}