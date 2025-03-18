#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;

in vec2 fs_uvs;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

void main() {
    vec3 hdrColor = texture(diffuseTexture, fs_uvs).rgb;
    // reinhard tone mapping for SDR workflow
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // TODO different tonemapping for actual HDR display, e.g. PQ, HLG

    color = vec4(mapped, 1.0);

    // Not sure about this, but as we are using potentially several srgb framebuffers now,
    // we must eventually map back so opengl can then do the sRGB correction by itself.
    float gamma = 2.2;
    //color.rgb = pow(color.rgb, vec3(gamma));

    float brightness = dot(hdrColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        brightColor = vec4(hdrColor.rgb, 1.0);
    else
        brightColor = vec4(0.0, 0.0, 0.0, 1.0);

}
