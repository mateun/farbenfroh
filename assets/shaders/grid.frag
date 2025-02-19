#version 450 core

uniform vec4 singleColor = vec4(0, 0.4, 0, 1);

in vec3 fragmentViewPos;
out vec4 color;

float getFogAmount() {
    float fogStart = 1;
    float fogRange = 30;
    return clamp(((-fragmentViewPos.z - fogStart) / fogRange), 0, 1);

}


void main() {
    color = singleColor;

    // Fog
    {
        vec3 fogColor = {0.3, 0.3, 0.2};
        color.xyz = mix(color.xyz, fogColor, getFogAmount());
    }

//    float maxDistance = 150;
//    float viewDistance = fsFogCameraPos.z * -1;
//    float distanceRatio = viewDistance / maxDistance;
//    color *= vec4(1,1,1, max(0.01, 1-distanceRatio));

}



