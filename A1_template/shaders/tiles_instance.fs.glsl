#version 330

// From vertex shader
in vec2 texcoord;
in vec4 sprite_loc; // x,y offsets; z,w scaling factor

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// Output color
layout(location = 0) out vec4 color;

void main()
{
    // Adapted from: https://gamedev.stackexchange.com/a/86356, https://gamedev.stackexchange.com/a/61800
    // Each texture in the atlas has 1 pixel border padding (clamped colors instead of transparent) to prevent seam artifacts
    // Or texture bleeding from adjacent textures
    // Another way to fix this is by setting GL_NEAREST instead of GL_LINEAR to prevent pixel interpolation
    // Size of one pixel
    // TODO: Create generic texture atlas class - current atlas size is hardcoded
    vec2 pixel = vec2(1.0) / vec2(306.0,238.0); 
    color = vec4(fcolor, 1.0) * texture(sampler0, texcoord * (sprite_loc.zw - pixel * 2.0) + sprite_loc.xy + pixel);
}