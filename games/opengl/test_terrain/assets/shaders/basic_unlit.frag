#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;

layout (location = 15) uniform float overrideAlpha = 1.0f;
layout (location = 16) uniform vec4 tint = vec4(1, 1, 1, 1);

uniform float lifeTime = 0;
uniform float frame;

in vec3 fsWorldPos;
in vec3 fsFogCameraPos;
in vec2 fs_uvs;
out vec4 color;



vec4 fog(vec4 c) {
    float z = length(fsFogCameraPos);
    float de = 0.025 * smoothstep(0, 6, 5 - fsWorldPos.y);
    float di = 0.045 * smoothstep(0, 40, 15 - fsWorldPos.y);
    float extinction = exp(-z * de);
    float inscattering = exp(-z * di);

    vec4 fogColor = vec4(0.9, 0.9, 0.9, 1);
    return c * extinction + fogColor * (1 - inscattering);

}

void main() {
    color = texture(diffuseTexture, fs_uvs);
    color.a *= overrideAlpha;
    color *= vec4(sin(frame* 0.05) + 0.5 , 0.1, cos(frame*0.03)+0.8, 1);
    //color.r = frame;

    //color = fog(color);

}
