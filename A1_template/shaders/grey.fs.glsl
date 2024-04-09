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
    float offsetX = end_pos.x - scale.x;
    float offsetY = end_pos.y - scale.y;

    vec2 constrainedTexcoord = vec2(texcoord.x * scale.x + offsetX, texcoord.y * scale.y + offsetY);

    vec4 sampledColor = texture(sampler0, constrainedTexcoord);

    float luminance = dot(sampledColor.rgb, vec3(0.299, 0.587, 0.114));
    color = vec4(luminance, luminance, luminance, sampledColor.a);
}