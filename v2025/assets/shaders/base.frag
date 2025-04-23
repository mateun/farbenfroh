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
uniform bool lit = false;
uniform vec4 singleColor = {1, 1, 1, 1};


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

bool isInShadowForPointLight(int lightIndex) {
    vec3 projCoords = fragPosPointLightSpace[lightIndex].xyz / fragPosPointLightSpace[lightIndex].w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMapsPoint[lightIndex], projCoords.xy).r;
    float currentDepth = projCoords.z;
    return currentDepth > (closestDepth + 0.004) ;
}


bool isInShadow(int lightIndex) {
    vec3 projCoords = fragPosLightSpace[lightIndex].xyz / fragPosLightSpace[lightIndex].w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMapsDir[lightIndex], projCoords.xy).r;
    float currentDepth = projCoords.z;
    return currentDepth > (closestDepth + 0.004) ;
}

vec4 calculateDirectionalLight(vec4 albedo, vec3 normal) {
    vec4 col = {0, 0, 0, 0};
    for (int i = 0; i < numDirectionalLights; i++) {
        vec3 tangentlightDir = normalize(tbn * -directionalLightData[i].direction) ;

        float diffuse = max(dot(normalize(normal), tangentlightDir), 0.2);
        col += vec4(albedo.xyz * diffuse, albedo.w);
        col.rgb *= directionalLightData[i].diffuseColor;
        if (isInShadow(i)) {
            col.rgb *= 0.3;
        }
    }

    return col;

}

vec4 calculateDirectionalLightWithoutNormalMap(vec4 albedo) {
    vec4 col = {0, 0, 0, 0};
    for (int i = 0; i < numPointLights; i++) {
        float diffuse = max(dot(normalize(fs_normals), -normalize(directionalLightData[i].direction)), 0.2);
        col += vec4(albedo.xyz * diffuse, albedo.w);
        if (isInShadow(i)) {
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

        float diffuse = max(dot(normalize(normal), tangentLightDir), 0.2);

        float distance = length(tangentLightPos - tangentFragPos);
        float att = 1.0 / (pointLightData[i].constant + pointLightData[i].linear * distance + pointLightData[i].quadratic * (distance * distance));
        diffuse *= att;
        vec4 accumuCol = vec4(albedo.xyz * diffuse, albedo.w);
        accumuCol.rgb *= pointLightData[i].diffuseColor;
        // TODO need a cubemap which the host renders into to represent omnidirectinal point light
        // shadows
        //        if (isInShadowForPointLight(i)) {
        //            accumuCol.rgb *= 0.3;
        //        }

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

subroutine vec4 calculateFinalColor(bool lit);
subroutine uniform calculateFinalColor colorFunctionUniform;

subroutine (calculateFinalColor)
vec4 calculateSingleColor(bool lit) {

    if (!lit) {
        return singleColor;
    } else {
        return vec4(0.5, 0, 0, 1);
    }

}

subroutine(calculateFinalColor)
vec4 calculateTextured(bool lit) {
    if (!lit) {
        return texture(diffuseTexture, fs_uvs);
    }else {
        vec4 col = vec4(0, 0, 0, 0);
        vec4 albedo = texture(diffuseTexture, fs_uvs);

        //Extract normal from map
        vec3 normal = texture(normalMap, fs_uvs).rgb;
        normal = normalize(normal * 2.0 - 1.0);

        col = calculateDirectionalLight(albedo, normal);
        //color = calculateDirectionalLightWithoutNormalMap(albedo);
        col += calculatePointLights(albedo, normal);
        return col;
    }
}

void main() {
    color = colorFunctionUniform(false);
    //color = vec4(1,0, 1,1);

}
