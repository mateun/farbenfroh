#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvs;

out vec2 fs_uvs;

void main() {
    gl_Position = vec4(pos,1 );
    fs_uvs = uvs;
    fs_uvs.y = 1- uvs.y;
}
