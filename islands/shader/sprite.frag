#version 330 core

uniform sampler2D tex;
uniform float alpha;
in vec2 uv;
out vec4 fragColor;

void main() {
    vec4 texColor = texture(tex, uv);
    fragColor = vec4(texColor.rgb, min(texColor.a, alpha));
}
