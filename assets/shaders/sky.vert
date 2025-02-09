#version 450 core

layout(location = 0) uniform mat4 mat_view;
layout(location = 1) uniform mat4 mat_proj;

out vec3 uv;

void main()
{
    vec3[4] vertices = vec3[4](vec3(-1, -1, 0),
                               vec3(1, -1, 0),
                                vec3(-1, 1, 0),
                                vec3(1,1, 0));


    //uv = normalize(mat3(mat_view) * vertices[gl_VertexID]);


    gl_Position = vec4(vertices[gl_VertexID], 1.0);
    uv = mat3(inverse(mat_view)) * (inverse(mat_proj) * gl_Position).xyz;







}

