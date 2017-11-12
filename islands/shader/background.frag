#version 330 core

in vec2 uv;
out vec3 fragColor;

void main() {
    vec3 a = vec3(227, 228, 233) / 255;
    vec3 b = vec3(0.529, 0.780, 0.898);
    fragColor = mix(a, b, uv.y);
}
