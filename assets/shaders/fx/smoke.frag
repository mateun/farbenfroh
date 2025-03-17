#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D shadowMap;

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
uniform float globalOpacity = 0.6;

in vec3 fs_normals;
in vec4 fragPosLightSpace[MAX_DIR_LIGHTS];
in vec4 fragPosPointLightSpace[MAX_POINT_LIGHTS];
in vec3 fsFogCameraPos;
in vec2 fs_uvs;
in vec4 tintInstanced;
in vec4 singleColorInstanced;
in float lifeFactor;
out vec4 color;

//bool isInShadow() {
//    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//    projCoords = projCoords * 0.5 + 0.5;
//    float closestDepth = texture(shadowMap, projCoords.xy).r;
//    float currentDepth = projCoords.z;
//
//    return currentDepth > (closestDepth + 0.004) ;
//}

vec4 calculateDirectionalLight(vec4 albedo, vec3 normal) {
    vec4 col = {0, 0, 0, 0};
    for (int i = 0; i < numDirectionalLights; i++) {
        vec3 tangentlightDir = normalize(-directionalLightData[i].direction) ;

        float diffuse = max(dot(normalize(normal), tangentlightDir), 0.2);
        col += vec4(albedo.xyz * diffuse, albedo.w);
        col.rgb *= directionalLightData[i].diffuseColor;
//        if (isInShadow(i)) {
//            col.rgb *= 0.3;
//        }
    }

    return col;

}


//vec4 calculatePointLights(vec4 albedo, vec3 normal) {
//
//    vec4 col = {0,0,0,0};
//    for (int i = 0; i < numPointLights; i++) {
//
//
//        vec3 lightPos = pointLightData[i].position;
//
//        // Next calculate the actual light direction from this fragment to the point light.
//        vec3 tangentLightDir = normalize(lightPos - tangentFragPos);
//
//        float diffuse = max(dot(normalize(normal), tangentLightDir), 0.2);
//
//        float distance = length(tangentLightPos - tangentFragPos);
//        float att = 1.0 / (pointLightData[i].constant + pointLightData[i].linear * distance + pointLightData[i].quadratic * (distance * distance));
//        diffuse *= att;
//        vec4 accumuCol = vec4(albedo.xyz * diffuse, albedo.w);
//        accumuCol.rgb *= pointLightData[i].diffuseColor;
//
//
//        col += accumuCol;
//    }
//
//    return col;
//
//}


void main() {
    if (true) {
        color = texture(diffuseTexture, fs_uvs);

        // Darken slightly with age
        color.rgb *= mix(1.0, 0.4, 1.0 - lifeFactor);

        float lifeTimeFade = smoothstep(0.0, 0.2, lifeFactor) * smoothstep(1.0, 0.8, lifeFactor);
        float verticalFade = smoothstep(0.0, 1.9, fs_uvs.y);

        color.a *= verticalFade * lifeTimeFade * globalOpacity;

    }else {
        vec4 col = vec4(0, 0, 0, 0);
        vec4 albedo = texture(diffuseTexture, fs_uvs);

        //Extract normal from map
//        vec3 normal = texture(normalMap, fs_uvs).rgb;
//        normal = normalize(normal * 2.0 - 1.0);
        vec3 normal = fs_normals;

        col = calculateDirectionalLight(albedo, normal);
        //col += calculatePointLights(albedo, normal);

        color = col;
    }




}
