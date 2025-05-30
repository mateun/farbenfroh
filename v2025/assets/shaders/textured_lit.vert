#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;
layout(location = 12) in vec3  tangent;

layout(location = 6) uniform mat4 mat_model;
layout(location = 7) uniform mat4 mat_view;
layout(location = 8) uniform mat4 mat_projection;
layout(location = 9) uniform mat4 mat_vp_directional_light;

layout(location = 18) uniform bool useNormals = true;

layout(location = 20) uniform bool flipUVs = false;
layout(location = 21) uniform float uvScale = 1;
layout(location = 22) uniform float uvPanX = 0;
layout(location = 23) uniform float uvPanY = 0;

layout(location = 25) uniform float uvScaleX = 1;
layout(location = 26) uniform float uvScaleY = 1;

out vec2 fs_uvs;
out vec3 fs_normals;
out vec3 fsFogCameraPos;
out vec4 fragPosLightSpace;
out vec4 vertexWeight;

out mat3 tbn;
out vec3 tangentFragPos;
out vec3 tangentViewPos;


void uvPanning();

void main() {

    gl_Position = mat_projection * mat_view * mat_model* vec4(pos, 1);
    fs_uvs = uvs;
    if (flipUVs) {
        fs_uvs.y = 1- uvs.y;
    }

    fs_uvs *= uvScale;
    uvPanning();


    vec3 nn = normalize(normals.xyz);
    vec4 normals_transformed = inverse(transpose(mat_model)) * vec4(nn, 1);
    fs_normals = normalize(normals_transformed.xyz);


    // Calculate the TBN matrix
    // And also calculate the tangentFragPos, we can do this here in the vertex shader
    vec3 T = normalize(vec3(mat_model * vec4(tangent, 0)));
    vec3 N = normalize(vec3(mat_model * vec4(normals, 0)));
    vec3 B = cross(N, T);
    tbn = transpose(mat3(T, B, N));
    tangentFragPos = tbn * vec3(mat_model * vec4(pos, 1.0));

    fsFogCameraPos = (mat_view * mat_model * vec4(pos,1)).xyz;
    fragPosLightSpace = mat_vp_directional_light * mat_model * vec4(pos, 1);

    vertexWeight = vec4(1, 1, 1, 1);
}

void uvPanning() {
    fs_uvs.x += uvPanX;
    fs_uvs.y += uvPanY;
}
