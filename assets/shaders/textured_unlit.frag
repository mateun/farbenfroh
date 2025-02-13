#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D shadowMap;
layout(binding = 2) uniform sampler2D normalMap;

layout (location = 15) uniform float overrideAlpha = 1.0f;
layout (location = 16) uniform vec4 tint = vec4(1, 1, 1, 1);

in vec2 fs_uvs;
out vec4 color;

void main() {
    color = texture(diffuseTexture, fs_uvs);
    color.a *= overrideAlpha;
    color *= tint;

}
