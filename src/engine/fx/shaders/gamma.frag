#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;

in vec2 fs_uvs;

out vec4 color;

void main() {
    color = texture(diffuseTexture, fs_uvs);
    float gamma = 2.2;
    color.rgb = pow(color.rgb, vec3(1.0/gamma));

}
