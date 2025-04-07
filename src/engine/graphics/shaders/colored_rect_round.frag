#version 450

uniform vec4 singleColor = {1, 1, 0, 1};
uniform vec4 gradientTargetColor = {1, 1, 0, 1};
uniform vec2 viewPortDimensions  = vec2(800, 600);
uniform vec2 viewPortOrigin = vec2(0, 0);

uniform vec2 rectSize;
uniform float radius = 8.0f;

in vec2 vLocalCoord;

out vec4 color;
in vec2 fs_uvs;

float sdRoundRect(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + vec2(r);
    return length(max(q, 0.0)) - r;
}

void main() {

    float t = (gl_FragCoord.y - viewPortOrigin.y) / viewPortDimensions.y;
    vec4 gradient = mix(gradientTargetColor.rgba, singleColor.rgba, t);
    vec4 uColor = vec4(gradient);

    vec2 vUV = fs_uvs * 2.0 - 1.0; //
    vec2 uSize = rectSize;
    float uRadius = radius;

    float dist = sdRoundRect(vUV * uSize * 0.5, uSize * 0.5, uRadius);
    float alpha = smoothstep(1.0, 0.0, dist); // Smooth edges (antialiasing)

    color = vec4(uColor.rgb, uColor.a * alpha);

}
