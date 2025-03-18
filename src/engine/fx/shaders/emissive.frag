#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;

uniform float emissionFactor = 1.0f;

in vec2 fs_uvs;

layout (location = 0) out vec4 color;

void main() {
    color = texture(diffuseTexture, fs_uvs);
    color.rgba *= emissionFactor;

}
