#version 330 core

uniform float time;

out vec4 fragColor;

void main() {
    const float duration = 1.0;
    fragColor = vec4(1, 1, 1, clamp(1.0 - time / duration, 0.0, 1.0));
}
