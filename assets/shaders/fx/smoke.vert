#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;
layout(location = 3) in int particleID;

// Particle structure matching the compute shader exactly!
struct Particle {
    vec4 position;   // xyz = world position
    vec4 velocity;   // if needed
    vec4 emitterPosition;
    float lifetime;
    int type;
    bool loop;
    vec2 padding;    // alignment
};

// Bind SSBO containing particles (must match compute shader binding)
layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};


uniform mat4 mat_world;
uniform mat4 mat_view;
uniform mat4 mat_projection;

uniform bool flipUVs = false;
uniform float uvScale = 1;
uniform float uvPanX = 0;
uniform float uvPanY = 0;

const int MAX_DIR_LIGHTS=2;
struct DirectionalLightData {
    vec3 direction;
    vec3 diffuseColor;
    mat4 mat_view_proj;
};
uniform DirectionalLightData directionalLightData[MAX_DIR_LIGHTS];
uniform int numDirectionalLights = 0;

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

// Uniforms for extracting camera vectors
uniform vec3 cameraRight;
uniform vec3 cameraUp;

out vec2 fs_uvs;
out vec3 fs_normals;
out vec3 fsFogCameraPos;
out vec3 fsWorldPos;
out vec4 fragPosLightSpace[MAX_DIR_LIGHTS];
out vec4 fragPosPointLightSpace[MAX_POINT_LIGHTS];
out vec4 singleColorInstanced;
out vec4 tintInstanced;
out float lifeFactor;

void uvPanning();

float random(float seed) {
    return fract(sin(seed) * 43758.5453123);
}

void main() {
    // Retrieve particle position directly from SSBO
    vec3 particleWorldPos = particles[int(particleID)].position.xyz;

    // Billboard alignment (camera-facing quad):
    float maxLifetime = 5.0f;
    lifeFactor = particles[particleID].lifetime / maxLifetime; // 0 (old) → 1 (new)
    float particleSize = mix(1.0, 3.5, 1-lifeFactor); // new particles smaller, older particles grow gently

    // Random rotation based on particleID:
    float angle = random(float(particleID)) * 6.2831;
    mat2 rotationMatrix = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    vec2 rotatedVertex = rotationMatrix * vec2(pos.x, pos.y);

    // Billboard offset

    vec3 vertexOffset = (cameraRight * rotatedVertex.x * particleSize* 1 + cameraUp * rotatedVertex.y * particleSize * 1.2) ;
    vec3 worldPos = particleWorldPos + vertexOffset;

    gl_Position = mat_projection * mat_view * vec4(worldPos + pos, 1.0);

    fs_uvs = uvs;
    if (flipUVs) {
        fs_uvs.y = 1- uvs.y;
    }

    fs_uvs *= uvScale;
    uvPanning();

    vec3 nn = normalize(normals.xyz);
    vec4 normals_transformed = inverse(transpose(mat_world)) * vec4(nn, 1);
    fs_normals = normals_transformed.xyz;
    fsFogCameraPos = (mat_view * vec4(worldPos,1)).xyz;
    fsWorldPos = gl_Position.xyz;

    for (int i = 0; i< numDirectionalLights; i++) {
        fragPosLightSpace[i] =  directionalLightData[i].mat_view_proj *  vec4(worldPos, 1);
    }

    for (int i = 0; i< numPointLights; i++) {
        fragPosPointLightSpace[i] =  pointLightData[i].mat_view_proj * vec4(worldPos, 1);
    }
}

void uvPanning() {
    fs_uvs.x += uvPanX;
    fs_uvs.y += uvPanY;
}
