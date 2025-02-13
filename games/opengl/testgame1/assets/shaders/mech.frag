#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D shadowMap;
layout(binding = 2) uniform sampler2D normalMap;

layout (location = 15) uniform float overrideAlpha = 1.0f;
layout (location = 16) uniform vec4 tint = vec4(1, 1, 1, 1);

struct DirectionalLightData {
    vec3 direction;
    vec3 diffuseColor;
    mat4 mat_view_proj;
};
uniform DirectionalLightData directionalLightData;

struct PointLightData {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    mat4 mat_view_proj;
};

#define NR_POINT_LIGHTS 4
// TODO use actual array for point lights
uniform PointLightData pointLightDatas;



in vec3 fs_normals;
in vec4 fragPosLightSpace;
in vec3 fsFogCameraPos;
in vec2 fs_uvs;
in vec3 viewPos;
in mat3 tbn;
in vec3 tangentFragPos;

out vec4 color;


bool isInShadow() {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    return currentDepth > (closestDepth + 0.004) ;
}

vec4 calculateDirectionalLight(vec4 albedo, vec3 normal) {
    vec3 tangentlightDir = normalize(tbn * directionalLightData.direction) * -1;


    float diffuse = max(dot(normalize(normal), tangentlightDir), 0.2);
    vec4 col  = vec4(albedo.xyz * diffuse, albedo.w);
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
    // TODO
    //color += calculatePointLight(albedo, normal);

    if (isInShadow()) {
        color *= 0.1;
    }

    color.a *= overrideAlpha;
    color *= tint;

    // TODO fog





}
