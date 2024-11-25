#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;
layout(location = 3) in vec2 instancedOffsets;

layout (location = 6) in vec4 instanceMatrix0;
layout (location = 7) in vec4 instanceMatrix1;
layout (location = 8) in vec4 instanceMatrix2;
layout (location = 9) in vec4 instanceMatrix3;

layout(location = 10) in vec4 instancedColors;
layout(location = 11) in vec4 instancedTints;

layout(location = 6) uniform mat4 mat_model;
layout(location = 7) uniform mat4 mat_view;
layout(location = 8) uniform mat4 mat_projection;
layout(location = 9) uniform mat4 mat_vp_directional_light;
layout(location = 18) uniform bool useNormals = true;

layout(location = 20) uniform bool flipUVs = false;
layout(location = 21) uniform float uvScale = 1;
layout(location = 22) uniform float uvPanX = 0;
layout(location = 23) uniform float uvPanY = 0;

out vec2 fs_uvs;
out vec3 fs_normals;
out vec3 fsFogCameraPos;
out vec4 fragPosLightSpace;
out vec4 vertexWeight;
out vec4 singleColorInstanced;
out vec4 tintInstanced;

void uvPanning();

void main() {
    //vec3 instancedPos = pos + vec3(instancedOffsets.x, 0, instancedOffsets.y);
    mat4 instanceMatrix = mat4(instanceMatrix0, instanceMatrix1, instanceMatrix2, instanceMatrix3);
    vec4 instancedPos = instanceMatrix * vec4(pos, 1);
    singleColorInstanced = instancedColors;
    tintInstanced = instancedTints;


    //gl_Position = mat_projection * mat_view * mat_model* vec4(instancedPos, 1);
    gl_Position = mat_projection * mat_view * instancedPos;
    fs_uvs = uvs;
    if (flipUVs) {
        fs_uvs.y = 1- uvs.y;
    }

    fs_uvs *= uvScale;
    uvPanning();

    if (useNormals) {
        vec3 nn = normalize(normals.xyz);
        vec4 normals_transformed = inverse(transpose(mat_model)) * vec4(nn, 1);
        fs_normals = normals_transformed.xyz;
    } else  {
        fs_normals = normals;
    }

    fsFogCameraPos = (mat_view * mat_model * instancedPos).xyz;

    fragPosLightSpace = mat_vp_directional_light * mat_model * instancedPos;
    vertexWeight = vec4(1, 1, 1, 1);
}

void uvPanning() {
    fs_uvs.x += uvPanX;
    fs_uvs.y += uvPanY;
}
