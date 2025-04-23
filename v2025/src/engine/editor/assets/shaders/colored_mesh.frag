#version 450


uniform vec4 singleColor;

const int MAX_DIR_LIGHTS = 2;
struct DirectionalLightData {
    vec3 direction;
    vec3 diffuseColor;
    mat4 mat_view_proj;
};
uniform DirectionalLightData directionalLightData[MAX_DIR_LIGHTS];
uniform int  numDirectionalLights = 0;

in vec3 fs_normals;
in vec3 fragmentViewPos;
out vec4 color;


vec4 calculateDirectionalLight(vec4 albedo, vec3 normal) {
    vec3 tangentlightDir = -directionalLightData[0].direction;

    float diffuse = max(dot(normalize(normal), tangentlightDir), 0.2);
    vec4 col  = vec4(albedo.xyz * diffuse, albedo.w);
    col *= vec4(directionalLightData[0].diffuseColor, 1);
    return col;

}

float getFogAmount() {
    float fogStart = 5;
    float fogRange = 40;
    return clamp(((fragmentViewPos.z - fogStart) / fogRange), 0, 1);

}


void main() {
    vec4 albedo =  singleColor;

    color = calculateDirectionalLight(albedo, fs_normals);

    //vec3 fogColor = {0.3, 0.3, 0.2};
    //color.xyz = mix(color.xyz, fogColor, getFogAmount());

}
