#version 330 core

uniform sampler2D lightmap;
in vec2 uv;
in vec3 normal;
in vec4 diffuseColor;
out vec3 color;

vec3 gamma(in vec3 c) {
    return pow(c, 2.2 * vec3(1, 1, 1));
}

vec3 invgamma(in vec3 c) {
    return pow(c, 1.0 / 2.2 * vec3(1, 1, 1));
}

void main() {
    color = texture2D(lightmap, uv).rgb;
}