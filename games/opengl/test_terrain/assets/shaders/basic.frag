#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D shadowMapsDir[2];
layout(binding = 3) uniform sampler2D shadowMapsPoint[6];
layout(binding = 9) uniform sampler2D shadowMapsSpot[4];
layout(binding = 13) uniform sampler2D normalMap;

layout (location = 15) uniform float overrideAlpha = 1.0f;
layout (location = 16) uniform vec4 tint = vec4(1, 1, 1, 1);

const int MAX_DIR_LIGHTS = 2;
struct DirectionalLightData {
    vec3 direction;
    vec3 diffuseColor;
    mat4 mat_view_proj;
};
uniform DirectionalLightData directionalLightData[MAX_DIR_LIGHTS];
uniform int  numDirectionalLights = 0;
uniform float shadowBias = 0.0;

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

in vec3 fs_normals;
in vec4 fragPosLightSpace[MAX_DIR_LIGHTS];
in vec4 fragPosPointLightSpace[MAX_POINT_LIGHTS];
in vec3 fsFogCameraPos;
in vec3 fsWorldPos;
in vec2 fs_uvs;
in vec3 viewPos;
in mat3 tbn;
in vec3 tangentFragPos;

out vec4 color;

vec4 shadowDistanceAsColor(int lightIndex) {
    vec3 projCoords = fragPosLightSpace[lightIndex].xyz / fragPosLightSpace[lightIndex].w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMapsDir[lightIndex], projCoords.xy).r;
    float currentDepth = projCoords.z;
    if (currentDepth - shadowBias > closestDepth) {
        return vec4(1, 0, 0, 1);
    } else {
        float diff = (currentDepth - shadowBias) * 10;
        return vec4(diff, diff, diff, 1);
    }

}

bool isInShadow(int lightIndex, float bias) {
    vec3 projCoords = fragPosLightSpace[lightIndex].xyz / fragPosLightSpace[lightIndex].w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMapsDir[lightIndex], projCoords.xy).r;
    float currentDepth = projCoords.z;
    // The smaller the depth range is (so, the less distance we have to span from near to far)
    // the smaller this bias must be.
    // The necessary bias must be smaller at least than the depth value you sample in renderdoc in the depth buffer
    // (as rendered by the shadowmap camera).
    return (currentDepth - bias) > closestDepth;
}

float getSlopeAdjustedBias(vec3 L, vec3 N) {
    // Adjust our bias based on th slope
    //float bias = max(0.0001 * (1.0 - dot(normalize(N), L)), shadowBias);
    float bias = 0.0001 + 0.003 * dot(normalize(N), L);
    return bias;;
    //return shadowBias;
}

vec4 calculateDirectionalLight(vec4 albedo, vec3 normal) {
    vec4 col = {0, 0, 0, 0};
    for (int i = 0; i < numDirectionalLights; i++) {
        vec3 tangentLightDir = normalize(tbn * -directionalLightData[i].direction) ;

        float diffuse = max(dot(normalize(normal), tangentLightDir), 0.2);
        col += vec4(albedo.xyz * diffuse, albedo.w);

        col.rgb *= directionalLightData[i].diffuseColor;
        float adjustedBias = getSlopeAdjustedBias(normal, tangentLightDir);
        if (isInShadow(i, adjustedBias)) {
            col.rgb *= 0.4;
        }

        //col = shadowDistanceAsColor(i);
    }

    return col;

}

vec4 calculateDirectionalLightWithoutNormalMap(vec4 albedo) {
    vec4 col = {0, 0, 0, 0};
    for (int i = 0; i < numPointLights; i++) {
        float diffuse = max(dot(normalize(fs_normals), -normalize(directionalLightData[i].direction)), 0.2);
        col += vec4(albedo.xyz * diffuse, albedo.w);
        float adjustedBias = getSlopeAdjustedBias(fs_normals, directionalLightData[i].direction);
        if (isInShadow(i, adjustedBias)) {
            col.rgb *= 0.3;
        }
    }

    return col;

}



vec4 calculatePointLights(vec4 albedo, vec3 normal) {

    vec4 col = {0,0,0,0};
    for (int i = 0; i < numPointLights; i++) {

        // First move the lightpos into the tangent space
        vec3 tangentLightPos = tbn * pointLightData[i].position;

        // Next calculate the actual light direction from this fragment to the point light.
        vec3 tangentLightDir = normalize(tangentLightPos - tangentFragPos);

        float diffuse = max(dot(normalize(normal), tangentLightDir), 0.4);

        float distance = length(tangentLightPos - tangentFragPos);
        float att = 1.0 / (pointLightData[i].constant + pointLightData[i].linear * distance + pointLightData[i].quadratic * (distance * distance));
        diffuse *= att;
        vec4 accumuCol = vec4(albedo.xyz * diffuse, albedo.w);
        accumuCol.rgb *= pointLightData[i].diffuseColor;
        col += accumuCol;
    }

    return col;

}

vec4 fog(vec4 c) {
    float z = length(fsFogCameraPos);
    float de = 0.025 * smoothstep(0, 6, 5 - fsWorldPos.y);
    float di = 0.045 * smoothstep(0, 40, 15 - fsWorldPos.y);
    float extinction = exp(-z * de);
    float inscattering = exp(-z * di);

    vec4 fogColor = vec4(0.9, 0.9, 0.9, 1);
    return c * extinction + fogColor * (1 - inscattering);

}

void main() {
    //color = vec4(0, 0, 0, 0);
    vec4 albedo = texture(diffuseTexture, fs_uvs);

    //Extract normal from map
    vec3 normal = texture(normalMap, fs_uvs).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    color = calculateDirectionalLight(albedo, normal);
    //color = calculateDirectionalLightWithoutNormalMap(albedo);
    color += calculatePointLights(albedo, normal);

    //color.rgb = normal;

    //color.a *= overrideAlpha;
    //color *= tint;

    //color = fog(color);






}
