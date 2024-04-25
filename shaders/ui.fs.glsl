#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform vec2 end_pos;
uniform vec2 scale;

// Output color
layout(location = 0) out  vec4 color;

void main()
{

    // Calculate offset based on the desired range
    float offsetX = end_pos.x-scale.x;
    float offsetY = end_pos.y-scale.y;

    // Apply scale and offset to texture coordinates
    vec2 constrainedTexcoord = vec2(texcoord.x * scale.x + offsetX, texcoord.y * scale.y + offsetY);

    // Sample the texture with constrained coordinates
    color = vec4(fcolor, 1.0) * texture(sampler0, constrainedTexcoord);
}