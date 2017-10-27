#version 330 core

uniform float time;
uniform vec4 diffuse;

out vec4 fragColor;

void main() {
    vec3 c = pow(diffuse.rgb, 0.4545 * vec3(1, 1, 1));
    fragColor = vec4(mix(c, vec3(1, 1, 1), 0.5 * cos(3.14 * time / 0.3) + 0.5), 1);
}