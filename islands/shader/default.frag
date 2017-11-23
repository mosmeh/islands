#version 330 core

uniform vec4 diffuse;
out vec4 fragColor;

void main() {
    fragColor = vec4(pow(diffuse.rgb, 0.4545 * vec3(1, 1, 1)), diffuse.a);
}
