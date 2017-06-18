#version 330 core

uniform sampler2D lightmap;
in vec2 uv;
out vec4 fragColor;

void main() {
    fragColor = texture(lightmap, uv);
}