#version 450

layout(binding = 0) uniform sampler2D scene;
layout(binding = 1) uniform sampler2D bloomBlur;

in vec2 fs_uvs;
in vec2 bloom_uvs;

out vec4 color;

void main() {
    vec3 hdrColor = texture(scene, fs_uvs).rgb;
    vec3 bloomColor = texture(bloomBlur, bloom_uvs).rgb;

    float bloomIntensity = 2.2;

    hdrColor += bloomColor * bloomIntensity;; // additive blending
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * 2);
    // for now no gamma...
    float gamma = 2.2f;
    result = pow(result, vec3(1.0 / gamma));
    color = vec4(hdrColor, 1.0);

}

