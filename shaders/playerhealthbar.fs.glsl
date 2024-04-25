#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float health_percentage;

// Output color
layout(location = 0) out vec4 color;

// Adapted from: https://stackoverflow.com/a/67268830, https://gamedev.stackexchange.com/a/204057
void main()
{
	// Origin is top left ?
	vec4 health_color;
	vec4 border_color;
	// hardcoded resolution width -> 194 pixels
	// 4 pixels from left edge of texture to left edge of health bar -> 70/194
	// 4 pixels from right edge of texture to right edge of health bar -> 78/194
	float left_border_size = 4.0/194.0;
	float right_border_size = 4.0/194.0;
	// to remove the left/right border padding, scale the health percentage
	right_border_size = (1.0 - left_border_size - right_border_size) * health_percentage + left_border_size;
	health_color = texture(sampler0, vec2(texcoord.x, texcoord.y / 2.0+0.5));
	border_color = texture(sampler0, vec2(texcoord.x, texcoord.y / 2.0));

	if (texcoord.x > left_border_size && texcoord.x < right_border_size) {
		color = health_color;
	} else {
		color = border_color;
	}
}