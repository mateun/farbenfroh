#version 450 core

layout(location = 0) uniform mat4 mat_view;

out vec3 uv;

void main()
{
    vec3[4] vertices = vec3[4](vec3(-1, -1, 1),
                               vec3(1, -1, 1),
                                vec3(-1, 1, 1),
                                vec3(1,1, 1));


    uv = mat3(mat_view) * vertices[gl_VertexID];

    gl_Position = vec4(vertices[gl_VertexID], 1.0);
}