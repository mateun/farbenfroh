#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;

layout(location = 9) uniform vec2 screen_size = vec2(1280, 720);
layout(location = 10) uniform float time = 10000;

in vec2 fs_uvs;
out vec4 color;

void applyFx() {

    // ---- 1) Barrel Distortion ----
    vec2 center = vec2(0.5, 0.5);
    vec2 coord = fs_uvs - center;
    float distFactor = 0.65 ; // curvature
    float r2 = dot(coord, coord);
    coord *= (1.0 + distFactor * r2);
    vec2 warpedUV = coord + center;

    // Bound check or clamp
    warpedUV = clamp(warpedUV, 0.0, 1.0);
    //warpedUV = fs_uvs;

    vec4 origColor = texture(diffuseTexture, fs_uvs);
    color = texture(diffuseTexture, warpedUV);


    float offset = 1.0 / screen_size.x * .6; // shift a fraction of a pixel

    vec3 colR = texture(diffuseTexture, fs_uvs + vec2( offset, 0.0)).rgb;
    vec3 colG = texture(diffuseTexture, fs_uvs).rgb;
    vec3 colB = texture(diffuseTexture, fs_uvs - vec2( offset, 0.0)).rgb;
    color = vec4(colR.r, colG.g, colB.b, color.a);


    // For reproducible noise, use a function of the fragment coords + time
    float n = fract(sin(dot(gl_FragCoord.xy ,vec2(12.9898,78.233))) * 43758.5453 + time);
    // Or any other noise approach
    float noiseAmount = 0.15;
    color.rgb += (n - 0.5) * noiseAmount;

    float flicker = 0.98 + 0.02 * sin(time * 5.0); // 120 Hz flicker for example
    color.rgb *= flicker;

    float distFromCenter = length(fs_uvs - vec2(0.5));
    float vignette = smoothstep(0.5, 0.8, distFromCenter);
    color *= (1.0 - 0.3 * vignette);

    if (int(mod(gl_FragCoord.y, 2)) == 0) {
        color *= 0.0;
    }

    color.a = 2 - (coord.y * 6);


    //color = color + origColor;

}

void main() {
    applyFx();

}
