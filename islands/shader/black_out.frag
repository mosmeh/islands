#version 330 core

uniform float progress;
uniform sampler2D tex;
in vec2 uv;
out vec3 color;

void main() {
    vec4 texColor = texture(tex, uv);
    if (progress < 0.5) {
        color = texColor.rgb * (0.5 - progress) * 2.0;
    } else if (progress < 1.0) {
        color = texColor.rgb * (progress - 0.5) * 2.0;
    } else {
        color = texColor.rgb;
    }
}