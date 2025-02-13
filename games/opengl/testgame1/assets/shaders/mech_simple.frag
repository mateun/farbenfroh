#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;

in vec2 fs_uvs;
in vec3 fs_normals;

out vec4 color;


void main() {

    vec4 albedo = texture(diffuseTexture, fs_uvs);
    color =  albedo;


}
