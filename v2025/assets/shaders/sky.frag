#version 450 core

out vec4 color;
in vec3 uv;

layout(binding = 0) uniform samplerCube skybox;

void main()
{
    color = texture(skybox, uv);
    //color = vec4(1, 0,1, 1);
}