#version 330 core

uniform vec4 diffuse;
uniform sampler2D tex;
in vec2 uv;
out vec4 fragColor;

void main() {
    vec4 texColor = texture(tex, uv);
    vec3 color = diffuse.rgb * mix(1, texColor.r, texColor.a);
    fragColor = vec4(pow(color, 0.4545 * vec3(1, 1, 1)), diffuse.a);
}
