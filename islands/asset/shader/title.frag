#version 330 core

uniform sampler2D tex;
uniform int selectedItem;
uniform float time;
in vec2 uv;
out vec4 fragColor;

void main() {
    vec4 texColor = texture(tex, uv);
    float y;
    if (selectedItem == 0) {
        y = 0.165;
    } else {
        y = 0.065;
    }
    if (y <= uv.y && uv.y <= y + 0.1) {
        float factor = 0.1 * sin(1.57 * time) + 0.5;
        fragColor = vec4(mix(texColor.rgb, vec3(1, 1, 1), factor), 1.0);
    } else {
        fragColor = texColor;
    }
}
