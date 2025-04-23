#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;

layout(location = 4) in ivec4 a_BoneIndices; // The indices of 4 bones
layout(location = 5) in vec4 a_BoneWeights;  // The influence of 4 bones (weights sum to 1)

layout(location = 6) uniform mat4 mat_world;
layout(location = 7) uniform mat4 mat_view;
layout(location = 8) uniform mat4 mat_projection;
layout(location = 18) uniform bool useNormals = true;


layout(location = 24) uniform mat4 u_BoneMatrices[100]; // All our bone global transform matrices

out vec2 fs_uvs;
out vec3 fs_normals;
out vec4 fragPosLightSpace;
out vec3 fragmentViewPos;

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


    gl_Position = mat_projection * mat_view * mat_world * vec4(skinnedPosition);

     if (useNormals) {
        vec4 normals_transformed = inverse(transpose(mat_world)) * normalize(skinnedNormals);
        fs_normals = normalize(normals_transformed.xyz);
     } else  {
        fs_normals = normalize(normals);
     }

    fs_uvs = uvs;

    fragmentViewPos = (mat_view * mat_world * skinnedPosition).xyz;


}

