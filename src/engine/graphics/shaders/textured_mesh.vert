#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvs;

uniform mat4 mat_world;
uniform mat4 mat_view;
uniform mat4 mat_projection;

out vec2 fs_uvs;


void main() {
    gl_Position = mat_projection * mat_view * mat_world* vec4(pos, 1);
    fs_uvs = uvs;
    //fs_uvs.y = 1 - fs_uvs.y;


}

