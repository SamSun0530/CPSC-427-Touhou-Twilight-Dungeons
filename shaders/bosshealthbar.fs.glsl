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
	// hardcoded resolution width -> 931 pixels
	// 70 pixels from left edge of texture to left edge of health bar -> 70/931
	// 78 pixels from right edge of texture to right edge of health bar -> 78/931
	float left_border_size = 70.0/931.0;
	float right_border_size = 78.0/931.0;
	// to remove the left/right border padding, scale the health percentage
	right_border_size = (1.0 - left_border_size - right_border_size) * health_percentage + left_border_size;
	health_color = texture(sampler0, vec2(texcoord.x, texcoord.y / 2.0));
	border_color = texture(sampler0, vec2(texcoord.x, texcoord.y / 2.0 + 0.5));

	if (texcoord.x > left_border_size && texcoord.x < right_border_size) {
		color = mix(health_color, border_color, border_color.a);
	} else {
		color = border_color;
	}
}

