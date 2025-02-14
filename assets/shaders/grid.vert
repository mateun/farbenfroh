#version 450 core

layout(location = 0) in vec3 pos;

uniform mat4 mat_model;
uniform mat4 mat_view;
uniform mat4 mat_projection;

void main() {
    gl_Position = mat_projection * mat_view * mat_model* vec4(pos, 1);
   // fsFogCameraPos = (mat_view * mat_model * vec4(pos,1)).xyz;
   // fragPosLightSpace = mat_vp_directional_light * mat_model * vec4(pos, 1);
}