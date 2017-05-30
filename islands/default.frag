#version 330 core

in vec4 diffuseColor;

void main() {
    gl_FragColor = vec4(pow(diffuseColor.rgb, 0.4545 * vec3(1, 1, 1)), 1);
}