#version 450

in vec3 fsWorldPos;

out vec4 position;

void main() {
    // For some reason this only works with a vec4 output setting the alpha to 1.
    // If outputting a vec3, it does not work at all, neither in renderdoc nor when reading back pixels
    // from the host.
    position = vec4(fsWorldPos, 1);

    // This does not work.. see above.
    //position = fsWorldPos;

    // Just a single color output for debugging in the shader.
    //position = vec4(0.5, 0, 0.5, 1);

}
