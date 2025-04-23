#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvs;

layout(location = 6) uniform mat4 mat_model;
layout(location = 7) uniform mat4 mat_view;
layout(location = 8) uniform mat4 mat_projection;


layout(location = 20) uniform bool flipUVs = false;
layout(location = 21) uniform float uvScale = 1;
layout(location = 22) uniform float uvPanX = 0;
layout(location = 23) uniform float uvPanY = 0;

layout(location = 25) uniform float uvScaleX = 1;
layout(location = 26) uniform float uvScaleY = 1;

out vec2 fs_uvs;
out vec3 fs_normals;


void uvPanning();

void main() {
    gl_Position = mat_projection * mat_view * mat_model* vec4(pos, 1);
    fs_uvs = uvs;
    if (flipUVs) {
        fs_uvs.y = 1- uvs.y;
    }

    fs_uvs *= uvScale;
    uvPanning();
}

void uvPanning() {
    fs_uvs.x += uvPanX;
    fs_uvs.y += uvPanY;
}
