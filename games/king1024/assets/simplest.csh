#version 450

// We define how many work items (invocations) exist per work group in X/Y/Z.
// Here we choose 1x1 for simplicity, meaning each invocation handles exactly 1 pixel.
layout (local_size_x = 1, local_size_y = 1) in;

// The output image we'll write to, bound to image unit 0.
// The format must match how we set up the texture (rgba32f, etc.).
//layout(rgba8, binding = 0) uniform image2D img_output;

layout (binding = 0, rgba32f) writeonly uniform image2D img_output;

void main()
{
    // The 2D coordinates of the current invocation in global space.
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);


    // Write a red color into the image at those coordinates.
    imageStore(img_output, pixelCoords, vec4(1.0, 1.0, 0.0, 1.0));






}
