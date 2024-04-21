#version 330
// This is adapted from lecture material (Wednesday Feb 28th 2024)

in vec2 TextCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform float transparency;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TextCoords).r);
    vec4 finalColor = vec4(textColor, 1.0) * sampled;
    finalColor.a *= transparency; // Adjust alpha with transparency
    color = finalColor;
}