#version 450

uniform vec4 singleColor = {1, 1, 0, 1};
uniform vec4 gradientTargetColor = {1, 1, 0, 1};
uniform float uScreenHeight  = 600;

out vec4 color;

void main() {


    float t = gl_FragCoord.y / uScreenHeight;

    vec3 gradient = mix(gradientTargetColor.rgb, singleColor.rgb, t);

    color = vec4(gradient, 1);

}
