#version 450

layout(location = 0) in vec3 pos;

uniform mat4 mat_world;
uniform mat4 mat_view;
uniform mat4 mat_projection;

out vec2 vLocalCoord;

void main() {
    gl_Position = mat_projection * mat_view * mat_world* vec4(pos, 1);


}

