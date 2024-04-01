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
    // Adapted from: https://gamedev.stackexchange.com/a/86356
    color = vec4(fcolor, 1.0) * texture(sampler0, texcoord * sprite_loc.zw + sprite_loc.xy);
}