#version 450

layout(location = 1) uniform vec4 singleColor = vec4(0, 0.4, 0, 1);

out vec4 color;

void main() {
    color = singleColor;

//    float maxDistance = 150;
//    float viewDistance = fsFogCameraPos.z * -1;
//    float distanceRatio = viewDistance / maxDistance;
//    color *= vec4(1,1,1, max(0.01, 1-distanceRatio));

}
