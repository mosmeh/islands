#version 330 core

#define NUM_MAX_BONES 128

uniform mat4 MVP;
uniform mat4 bones[NUM_MAX_BONES];

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in ivec4 boneIDs;
layout(location = 4) in vec4 weights;

out vec2 uv;
out vec3 normal;

void main() {
    mat4 boneTransform =
        bones[boneIDs[0]] * weights[0] +
        bones[boneIDs[1]] * weights[1] +
        bones[boneIDs[2]] * weights[2] +
        bones[boneIDs[3]] * weights[3];

    gl_Position = MVP * (boneTransform * vec4(pos, 1));
    uv = in_uv;
    normal = (boneTransform * vec4(normal, 0)).xyz;
}
