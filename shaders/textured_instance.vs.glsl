#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;
in mat3 in_transform;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 projection;
uniform mat3 view;

void main()
{
	texcoord = in_texcoord;
	vec3 pos = projection * view * in_transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}