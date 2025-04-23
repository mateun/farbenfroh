#version 450

layout(binding = 0) uniform sampler2D scene;
layout(binding = 1) uniform sampler2D bloomBlur;

in vec2 fs_uvs;
in vec2 bloom_uvs;

out vec4 color;

/*
* Adding the bright bloom parts to the original image to have glow effect.
*/
void main() {
    vec3 hdrColor = texture(scene, fs_uvs).rgb;
    vec3 bloomColor = texture(bloomBlur, bloom_uvs).rgb;

    float bloomIntensity = 2.7f;
    bloomColor.rgb *= bloomIntensity; // TODO make tint parameterized?

    hdrColor += bloomColor; // additive blending

    color = vec4(hdrColor, 1.0);

}

