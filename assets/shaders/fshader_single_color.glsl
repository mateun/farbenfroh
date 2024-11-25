#version 450

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D shadowMap;

layout(location = 1) uniform vec4 singleColor = vec4(0, 0.4, 0, 1);

layout (location = 10) uniform vec3 lightDirection = { -0.1, -1, -0.1 };
layout (location = 11) uniform vec3 lightColor = { 1, 1, 1 };
layout (location = 12) uniform vec3 ambientColor = { 0.7, 0.7, 0.4 };
layout (location = 13) uniform bool lit = true;
layout (location = 14) uniform bool debugShadowMap = false;
layout (location = 15) uniform float overrideAlpha = 1.0f;
layout (location = 16) uniform vec4 tint = vec4(1, 1, 1, 1);

in vec3 fs_normals;
in vec4 fragPosLightSpace;
in vec3 fsFogCameraPos;
in vec4 vertexWeight;
out vec4 color;

// This outputs a color band depending on
// the z value of the shadow map
void drawShadowMapBands(float closestDepth) {

    if (closestDepth < 1.0) {
        color = vec4(0, 0.9, 0, 1);
    }

    if (closestDepth < 0.4) {
        color = vec4(0, 0.6, 0.4, 1);
    }
    if (closestDepth < 0.03) {
        color = vec4(0, 0.5, 0, 1);
    }
    if (closestDepth < 0.02) {
        color = vec4(0.4, 0.4, 0, 1);
    }
    if (closestDepth < 0.01) {
        color = vec4(0.5, 0.2, 0, 1);
    }
    if (closestDepth < 0.005) {
        color = vec4(0.5, 0.0, 0, 1);
    }

}

void displayShadowMapArea(float closestDepth) {
    // Smaller 1.0 means we are actually in the shadow map.
    if (closestDepth < 1.0) {
        color *= vec4(0.2, 1, 0.2, 1);
    }
}

bool isInShadow() {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    if (debugShadowMap) {
        drawShadowMapBands(closestDepth);
    }

    //displayShadowMapArea(closestDepth);

    return currentDepth > (closestDepth + 0.004) ;
}

void diffuseLighting(vec4 baseColor) {
    if (lit) {
        vec3 lightVector = normalize(lightDirection)*-1;
        float diffuse = max(dot(normalize(fs_normals), lightVector), 0.3);
        color  = vec4(baseColor.xyz * diffuse, baseColor.w);
        if (isInShadow()) {
            color.rgb *= 0.5;
        }
    }
}

void main() {
    color = singleColor;
    diffuseLighting(color);
    color *= tint;

    float maxDistance = 150;
    float viewDistance = fsFogCameraPos.z * -1;
    float distanceRatio = viewDistance / maxDistance;
    color *= vec4(1,1,1, max(0.01, 1-distanceRatio));




}
