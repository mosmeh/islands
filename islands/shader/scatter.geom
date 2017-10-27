#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 VP;
uniform float time;

in vec3 worldPos[3];
in vec3 normal[3];

void main() {
    vec3 triangleNormal = vec3(0, 0, 0);
    vec3 center = vec3(0, 0, 0);
    for (int i = 0; i < 3; ++i) {
        triangleNormal += normalize(normal[i]);
        center += worldPos[i];
    }
    triangleNormal /= 3;
    center /= 3;

    float c = cos(time);
    float s = sin(time);
    mat3 rot = mat3(c, -s, c, -s, c, c, s, -s, s);

    for (int i = 0; i < 3; ++i) {
        vec3 rotatedPos = center + rot * (worldPos[i] - center);
        gl_Position = VP * vec4(rotatedPos + 2 * time * triangleNormal, 1);
        EmitVertex();
    }
    EndPrimitive();
}