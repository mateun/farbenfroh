#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvs;

layout(location = 6) uniform mat4 mat_model;
layout(location = 7) uniform mat4 mat_view;
layout(location = 8) uniform mat4 mat_projection;

layout(location = 21) uniform float uvScale = 1;

out vec2 fs_uvs;

void main() {

    fs_uvs = uvs;
    fs_uvs.y = 1- uvs.y;
    fs_uvs *= uvScale;

    gl_Position = mat_projection * mat_view * mat_model* vec4(pos, 1);

}
