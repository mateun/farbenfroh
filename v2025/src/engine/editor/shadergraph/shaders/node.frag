#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;

out vec4 color;
in vec2 fs_uvs;


void main() {
    color = texture(diffuseTexture, fs_uvs);
    //color = vec4(1, 1, 0, 1);

}
