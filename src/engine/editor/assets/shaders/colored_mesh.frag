#version 450


uniform vec4 singleColor;

in vec3 fs_normals;

out vec4 color;


void main() {
    color =  singleColor;

}
