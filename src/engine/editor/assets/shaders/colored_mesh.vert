#version 450

layout(location = 0) in vec3 pos;
layout(location = 2) in vec3 normals;

layout(location = 6) uniform mat4 mat_world;
layout(location = 7) uniform mat4 mat_view;
layout(location = 8) uniform mat4 mat_projection;


out vec3 fs_normals;
out vec3 fragmentViewPos;


void main() {

    gl_Position = mat_projection * mat_view * mat_world* vec4(pos, 1);

    vec3 nn = normalize(normals.xyz);
    vec4 normals_transformed = inverse(transpose(mat_world)) * vec4(nn, 1);
    fs_normals = normalize(normals_transformed.xyz);

    fragmentViewPos = (mat_view * mat_world * vec4(pos,1)).xyz;


}

