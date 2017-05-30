#version 330 core

uniform sampler2D lightmap;
in vec2 uv;

void main() {
    gl_FragColor = texture2D(lightmap, uv);
}