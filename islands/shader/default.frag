#version 330 core

in vec4 diffuseColor;
out vec4 fragColor;

void main() {
    fragColor = vec4(pow(diffuseColor.rgb, 0.4545 * vec3(1, 1, 1)), 1);
}