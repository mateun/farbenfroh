#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;

uniform vec4 textColor;

in vec2 fs_uvs;

out vec4 color;

void main() {
    float r =  texture(diffuseTexture, fs_uvs).r;
    color = vec4(textColor.r, textColor.g , textColor.b, r);

}
