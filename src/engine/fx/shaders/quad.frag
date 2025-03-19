#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;

// 0 = filmic
// 1 = reinhard
uniform int toneMapping = 0;

in vec2 fs_uvs;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

void main() {
    vec3 hdrColor = texture(diffuseTexture, fs_uvs).rgb;

    // SDR workflow
    vec3 filmicMapped = vec3(1.0) - exp(-hdrColor * 2);
    vec3 reinhardMapped = hdrColor / (hdrColor + vec3(1.0));
    vec3[] mappings = {filmicMapped, reinhardMapped};
    // TODO different tonemapping for actual HDR display, e.g. PQ, HLG

    color = vec4(mappings[toneMapping], 1.0);

    float brightness = dot(hdrColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        brightColor = vec4(hdrColor.rgb, 1.0);
    else
        brightColor = vec4(0.0, 0.0, 0.0, 1.0);

}
