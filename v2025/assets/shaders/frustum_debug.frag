#version 450

uniform vec4 singleColor = vec4(1, 0, 1, 1);
out vec4 color;

void main() {
    color = singleColor;
}
