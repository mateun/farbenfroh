#version 450

layout(location = 0) in vec3 pos;


uniform mat4 mat_view;
uniform mat4 mat_projection;

void main() {
    gl_Position = mat_projection * mat_view * vec4(pos, 1);
}
