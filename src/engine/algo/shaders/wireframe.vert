#version 450 core

layout(location = 0) in vec3 pos;

layout(location = 6) uniform mat4 mat_world;
layout(location = 7) uniform mat4 mat_view;
layout(location = 8) uniform mat4 mat_projection;


void main() {

    gl_Position = mat_projection * mat_view * mat_world * vec4(pos, 1);


}

