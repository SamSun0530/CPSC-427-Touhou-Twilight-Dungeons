#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;
uniform float bomb_screen_factor;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv) 
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// A1: HANDLE THE WIND DISTORTION HERE (you may want to try sin/cos)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	return uv;
}

vec4 color_shift(vec4 in_color) 
{ 
    float effect_strength = 0.0;
    if (bomb_screen_factor > 0) {
        vec2 center = vec2(0.5, 0.5);
        float max_distance = distance(vec2(0.5, 0.5), vec2(1.0, 1.0));
        float current_distance = distance(texcoord, center);
        float normalized_distance = current_distance / max_distance;

        float phase1_max = 0.5;

        float white_effect = smoothstep(0.0, phase1_max, 2.0*bomb_screen_factor * (1.0 - normalized_distance));
        float recovery_start = phase1_max;
        float recovery_effect = 0.0;
        if (bomb_screen_factor > recovery_start) {
            float recovery_factor = (bomb_screen_factor - recovery_start) / (1.0 - recovery_start);
            recovery_effect = smoothstep(0.0, 1.0,2.0* recovery_factor * (1.0 - normalized_distance));
        }

        effect_strength = max(0.0, white_effect - recovery_effect);
    }
    return mix(in_color, vec4(0.8, 0.8, 0.8, 1.0), effect_strength);
}

vec4 fade_color(vec4 in_color) 
{
	if (darken_screen_factor > 0)
		in_color -= darken_screen_factor * vec4(0.8, 0.8, 0.8, 0);
	return in_color;
}

void main()
{
	vec2 coord = distort(texcoord);

    vec4 in_color = texture(screen_texture, coord);
    color = color_shift(in_color);
    color = fade_color(color);
}