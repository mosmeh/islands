#version 330 core

#define NUM_MAX_BONES 128

uniform mat4 M;
uniform mat4 MV;
uniform mat4 bones[NUM_MAX_BONES];

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 in_normal;
layout(location = 3) in ivec4 boneIDs;
layout(location = 4) in vec4 weights;

out vec3 worldPos;
out vec3 normal;

void main() {
    mat4 boneTransform =
        bones[boneIDs[0]] * weights[0] +
        bones[boneIDs[1]] * weights[1] +
        bones[boneIDs[2]] * weights[2] +
        bones[boneIDs[3]] * weights[3];

    vec4 transformed = boneTransform * vec4(pos, 1);
    worldPos = (M * transformed).xyz;
    normal = (MV * (boneTransform * vec4(in_normal, 0))).xyz;
}
