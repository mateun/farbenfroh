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
uniform PointLightData pointLightData;

struct TangentLightData {
    vec3 lightDir;
    vec3 lightPos;
    vec3 viewPos;
    vec3 fragPos;
};

in vec3 fs_normals;
in vec4 fragPosLightSpace;
in vec3 fsFogCameraPos;
in vec2 fs_uvs;
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

void diffuseLighting(vec4 baseColor, vec3 normal) {

    // In the case of a point light we calculate some radial falloff:
    if (isPointLight) {
        vec3 lightVector = normalize(fs_in.tangentLightPos - fs_in.tangentFragPos);
        float diffuse = max(dot(normalize(normal), lightVector), 0.2);
        color  = vec4(baseColor.xyz * diffuse, baseColor.w);
        float distance = length(fs_in.tangentLightPos - fs_in.tangentFragPos);
        float att = 1.0 / (pointLightConstant + pointLightLinear * distance + pointLightQuadratic * (distance * distance));
        color.rgb *= att;

    } else {
        vec3 lightVector = normalize(-fs_in.tangentLightDir);
        float diffuse = max(dot(normalize(normal), lightVector), 0.2);
        color  = vec4(baseColor.xyz * diffuse, baseColor.w);
        color  = vec4(baseColor.xyz * diffuse, baseColor.w);
    }

    if (isInShadow()) {
        color.rgb *= 0.2;
    }

}

vec4 calculateDirectionalLight(vec4 albedo, vec3 normal) {
    vec3 lightDir = normalize(tbn * directionalLightData.direction) * -1;
    //tld.viewPos = tbn * viewPos;

    float diffuse = max(dot(normalize(normal), lightDir), 0.2);
    vec4 col  = vec4(albedo.xyz * diffuse, baseColor.w);
    return col;

}

void calculatePointLight(vec4 albedo, vec3 normal) {

}

void main() {

    vec4 albedo = texture(diffuseTexture, fs_uvs);

    //Extract normal from map
    vec3 normal = texture(normalMap, fs_uvs).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal *= 1;

    color = calculateDirectionalLight(albedo, normal);
    //color += calculatePointLight(albedo, normal);

    color.a *= overrideAlpha;
    color *= tint;

    // TODO fog



}
