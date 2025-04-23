#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;
layout(location = 12) in vec3  tangent;

uniform mat4 mat_world;
uniform mat4 mat_view;
uniform mat4 mat_projection;

uniform float uvScale = 1;
uniform float uvPanX = 0;
uniform float uvPanY = 0;

const int MAX_DIR_LIGHTS=2;
struct DirectionalLightData {
    vec3 direction;
    vec3 diffuseColor;
    mat4 mat_view_proj;
};
uniform DirectionalLightData directionalLightData[MAX_DIR_LIGHTS];
uniform int numDirectionalLights = 0;

struct PointLightData {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 diffuseColor;
    mat4 mat_view_proj;
};
const int MAX_POINT_LIGHTS = 6;
uniform PointLightData pointLightData[MAX_POINT_LIGHTS];
uniform int numPointLights = 0;

out vec2 fs_uvs;
out vec3 fs_normals;
out vec3 fsFogCameraPos;
out vec3 fsWorldPos;
out vec4 fragPosLightSpace[MAX_DIR_LIGHTS];
out vec4 fragPosPointLightSpace[MAX_POINT_LIGHTS];

out mat3 tbn;
out vec3 tangentFragPos;
out vec3 tangentViewPos;

void uvPanning();

void main() {

    gl_Position = mat_projection * mat_view * mat_world* vec4(pos, 1);
    fs_uvs = uvs;
    fs_uvs.y = 1- uvs.y;

    fs_uvs *= uvScale;
    uvPanning();

    vec3 nn = normalize(normals.xyz);
    vec4 normals_transformed = inverse(transpose(mat_world)) * vec4(nn, 1);
    fs_normals = normalize(normals_transformed.xyz);

    // Calculate the TBN matrix
    // And also calculate the tangentFragPos, we can do this here in the vertex shader
    vec3 T = normalize(vec3(mat_world * vec4(tangent, 0)));
    vec3 N = normalize(vec3(mat_world * vec4(normals, 0)));
    vec3 B = cross(N, T);
    tbn = transpose(mat3(T, B, N));
    tangentFragPos = tbn * vec3(mat_world * vec4(pos, 1.0));

    fsFogCameraPos = (mat_view * mat_world * vec4(pos,1)).xyz;
    fsWorldPos = gl_Position.xyz;

    for (int i = 0; i< numDirectionalLights; i++) {
        fragPosLightSpace[i] =  directionalLightData[i].mat_view_proj * mat_world * vec4(pos, 1);
    }

    for (int i = 0; i< numPointLights; i++) {
        fragPosPointLightSpace[i] =  pointLightData[i].mat_view_proj * mat_world * vec4(pos, 1);
    }


}

void uvPanning() {
    fs_uvs.x += uvPanX;
    fs_uvs.y += uvPanY;
}
