#version 450 core
layout (location = 0) in vec3 aPos;

layout(location = 4) uniform mat4 mat_model;
layout(location = 5) uniform mat4 mat_view;
layout(location = 6) uniform mat4 mat_projection;

out vec3 uv;

void main()
{
    uv = aPos;
    mat4 view = mat4(mat3(mat_view));
    gl_Position = mat_projection * view * vec4(aPos, 1.0);
}