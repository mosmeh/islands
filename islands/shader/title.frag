#version 330 core

uniform sampler2D tex;
uniform int selectedItem;
in vec2 uv;
out vec4 fragColor;

void main() {
    vec4 texColor = texture(tex, uv);
    float y;
    if (selectedItem == 0) {
        y = 0.065;
    } else {
        y = 0.165;
    }
    if (y <= uv.y && uv.y <= y + 0.1) {
        fragColor = vec4(mix(texColor.rgb, vec3(1, 1, 1), 0.5), 1.0);
    } else {
        fragColor = texColor;
    }
}