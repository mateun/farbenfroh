#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;
layout(location = 4) in ivec4 a_BoneIndices; // The indices of 4 bones
layout(location = 5) in vec4 a_BoneWeights;  // The influence of 4 bones (weights sum to 1)

layout(location = 12) in vec3  tangent;

uniform mat4 mat_world;
uniform mat4 mat_view;
uniform mat4 mat_projection;
uniform mat4 u_BoneMatrices[100]; // All our bone global transform matrices

uniform float uvScale = 1;
uniform float uvPanX = 0;
uniform float uvPanY = 0;

const int NUM_DIR_LIGHTS=2;
struct DirectionalLightData {
    vec3 direction;
    vec3 diffuseColor;
    mat4 mat_view_proj;
};
uniform DirectionalLightData directionalLightData[NUM_DIR_LIGHTS];

out vec2 fs_uvs;
out vec3 fs_normals;
out vec3 fsFogCameraPos;
out vec4 fragPosLightSpace[NUM_DIR_LIGHTS];

out mat3 tbn;
out vec3 tangentFragPos;
out vec3 tangentViewPos;

void uvPanning();

void main() {

    // Transform the position by the bone matrices
    vec4 skinnedPosition = vec4(0.0);
    vec4 skinnedNormals = vec4(0.0);
    int skinned = 0;

    // Iterate over the 4 influencing bones
    for (int i = 0; i < 4; i++) {
        int boneIndex = a_BoneIndices[i];

        if (boneIndex != -1) {
            skinned = 1;
            mat4 boneMatrix = u_BoneMatrices[boneIndex]; // Fetch the bone's matrix
            skinnedPosition += boneMatrix * vec4(pos, 1.0) * a_BoneWeights[i];

            // To correctly transform normals, use the inverse transpose of the model matrix
            vec3 nn = normalize(normals.xyz);
            skinnedNormals += boneMatrix  * vec4(nn, 0) * a_BoneWeights[i];

        }

        fs_normals = vec3(normalize(skinnedNormals).xyz);
    }

    // No bone influence
    if (skinned == 0) {
        skinnedPosition = vec4(pos, 1);
        skinnedNormals = vec4(normals, 0);
    }

    gl_Position = mat_projection * mat_view * mat_world * skinnedPosition;
    fs_uvs = uvs;
    fs_uvs.y = 1- uvs.y;

    fs_uvs *= uvScale;
    uvPanning();


    // Calculate the TBN matrix
    // And also calculate the tangentFragPos, we can do this here in the vertex shader
    vec3 T = normalize(vec3(mat_world * vec4(tangent, 0)));
    vec3 N = normalize(vec3(mat_world * skinnedNormals));
    vec3 B = cross(N, T);
    tbn = transpose(mat3(T, B, N));
    tangentFragPos = tbn * vec3(mat_world * skinnedPosition);

    fsFogCameraPos = (mat_view * mat_world * skinnedPosition).xyz;
    for (int i = 0; i< NUM_DIR_LIGHTS; i++) {
        fragPosLightSpace[i] =  directionalLightData[i].mat_view_proj * mat_world * skinnedPosition;
    }

}

void uvPanning() {
    fs_uvs.x += uvPanX;
    fs_uvs.y += uvPanY;
}
