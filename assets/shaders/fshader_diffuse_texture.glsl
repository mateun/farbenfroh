#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D shadowMap;

layout (location = 10) uniform vec3 lightDirection = { -1.0, -0, 0 };
layout (location = 11) uniform vec3 lightColor = { 1, 1, 1 };
layout (location = 12) uniform vec3 ambientColor = { 0.2, 0.2, 0.2 };
layout (location = 13) uniform bool lit = true;
layout (location = 14) uniform bool debugShadowMap = false;
layout (location = 15) uniform float overrideAlpha = 1.0f;
layout (location = 16) uniform vec4 tint = vec4(1, 1, 1, 1);

in vec3 fs_normals;
in vec4 fragPosLightSpace;
in vec3 fsFogCameraPos;
in vec2 fs_uvs;
out vec4 color;

bool isInShadow() {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    return currentDepth > (closestDepth + 0.004) ;
}


void diffuseLighting(vec4 baseColor) {
    if (lit) {
        vec3 lightVector = normalize(-lightDirection);
        float diffuse = max(dot(normalize(fs_normals), lightVector), 0.1);
        color  = vec4(baseColor.xyz * diffuse, baseColor.w);
        if (isInShadow()) {
            color.rgb *= 0.1;
        }
    }
}

void main() {
    color = texture(diffuseTexture, fs_uvs);
    diffuseLighting(color);
    color.a *= overrideAlpha;
    color *= tint;


    if (lit) {
        float maxDistance = 70;
        float viewDistance = fsFogCameraPos.z * -1;
        float distanceRatio = viewDistance / maxDistance;
        //color *= vec4(0.3,0.3,0.9, 1) * distanceRatio;
    }




}
