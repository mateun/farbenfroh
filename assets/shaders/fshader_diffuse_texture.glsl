#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D shadowMap;
layout(binding = 2) uniform sampler2D normalMap;


layout (location = 11) uniform vec3 lightColor = { 1, 1, 1 };
layout (location = 12) uniform vec3 ambientColor = { 0.2, 0.2, 0.2 };
layout (location = 13) uniform bool lit = true;
layout (location = 14) uniform bool debugShadowMap = false;
layout (location = 15) uniform float overrideAlpha = 1.0f;
layout (location = 16) uniform vec4 tint = vec4(1, 1, 1, 1);

uniform vec3 lightSpecular;
uniform float pointLightConstant = 1.0f;
uniform float pointLightLinear = 0.08f;
uniform float pointLightQuadratic = .01f;
uniform bool isPointLight = false;

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;

};

in vec3 fs_normals;
in vec4 fragPosLightSpace;
in vec3 fsFogCameraPos;
in vec2 fs_uvs;
out vec4 color;

in VS_OUT {
    vec3 tangentLightDir;
    vec3 tangentLightPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
} fs_in;

bool isInShadow() {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    return currentDepth > (closestDepth + 0.004) ;
}


void diffuseLighting(vec4 baseColor, vec3 normal) {

    if (lit) {



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
}

void main() {
    color = texture(diffuseTexture, fs_uvs);

    // Extract normal from map
    vec3 normal = texture(normalMap, fs_uvs).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal *= 1;

    diffuseLighting(color, normal);
    color.a *= overrideAlpha;
    color *= tint;

    if (lit) {
        float maxDistance = 70;
        float viewDistance = fsFogCameraPos.z * -1;
        float distanceRatio = viewDistance / maxDistance;
        //color *= vec4(0.3,0.3,0.9, 1) * distanceRatio;
    }




}
