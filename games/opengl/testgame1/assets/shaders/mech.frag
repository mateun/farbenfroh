#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D shadowMapsDir[2];
layout(binding = 3) uniform sampler2D shadowMapsPoint[6];
layout(binding = 9) uniform sampler2D shadowMapsSpot[4];
layout(binding = 13) uniform sampler2D normalMap;

layout (location = 15) uniform float overrideAlpha = 1.0f;
layout (location = 16) uniform vec4 tint = vec4(1, 1, 1, 1);

const int NUM_DIR_LIGHTS = 2;
struct DirectionalLightData {
    vec3 direction;
    vec3 diffuseColor;
    mat4 mat_view_proj;
};
uniform DirectionalLightData directionalLightData[NUM_DIR_LIGHTS];

struct PointLightData {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    mat4 mat_view_proj;
};

#define NUM_POINT_LIGHTS 6
uniform PointLightData pointLightDatas[NUM_POINT_LIGHTS];


in vec3 fs_normals;
in vec4 fragPosLightSpace[NUM_DIR_LIGHTS];
in vec3 fsFogCameraPos;
in vec2 fs_uvs;
in vec3 viewPos;
in mat3 tbn;
in vec3 tangentFragPos;

out vec4 color;


bool isInShadow(int lightIndex) {
    vec3 projCoords = fragPosLightSpace[lightIndex].xyz / fragPosLightSpace[lightIndex].w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMapsDir[lightIndex], projCoords.xy).r;
    float currentDepth = projCoords.z;
    return currentDepth > (closestDepth + 0.004) ;
}

vec4 calculateDirectionalLight(vec4 albedo, vec3 normal) {
    vec4 col = albedo;
    for (int i = 0; i < NUM_DIR_LIGHTS; i++) {
        vec3 tangentlightDir = normalize(tbn * -directionalLightData[i].direction) ;

        float diffuse = max(dot(normalize(normal), tangentlightDir), 0.2);
        col  += vec4(albedo.xyz * diffuse, albedo.w);
        col *= vec4(directionalLightData[i].diffuseColor, 1);
        if (isInShadow(i)) {
            col *= 0.3;
        }
    }

    return col;

}

vec4 calculateDirectionalLightWithoutNormalMap(vec4 albedo) {
    vec4 col = albedo;
    for (int i = 0; i < NUM_DIR_LIGHTS; i++) {
        float diffuse = max(dot(normalize(fs_normals), -normalize(directionalLightData[i].direction)), 0.2);
        col += vec4(albedo.xyz * diffuse, albedo.w);
        if (isInShadow(i)) {
            col *= 0.1;
        }
    }

    return col;

}



vec4 calculatePointLight(vec4 albedo, vec3 normal, PointLightData pointLightData) {
    // First move the lightpos into the tangent space
    vec3 tangentLightPos = tbn * pointLightData.position;

    // Next calculate the actual light direction from this fragment to the point light.
    vec3 tangentLightDir = normalize(tangentLightPos - tangentFragPos);

    float diffuse = max(dot(normalize(normal), tangentLightDir), 0.2);
    vec4 col  = vec4(albedo.xyz * diffuse, albedo.w);
    float distance = length(tangentLightPos - tangentFragPos);
    float att = 1.0 / (pointLightData.constant + pointLightData.linear * distance + pointLightData.quadratic * (distance * distance));
    return col * att;

}

void main() {

    color = vec4(1, 0, 1, 1);


    vec4 albedo = texture(diffuseTexture, fs_uvs);

    //Extract normal from map
    vec3 normal = texture(normalMap, fs_uvs).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    color = calculateDirectionalLight(albedo, normal);
    //color = calculateDirectionalLightWithoutNormalMap(albedo);
    // TODO
    //color += calculatePointLight(albedo, normal);



    color.a *= overrideAlpha;
    color *= tint;

    // TODO fog

    //color = vec4(1, 0,1, 1);



}
