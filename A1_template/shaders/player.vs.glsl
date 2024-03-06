#version 330

// Input attributes
in vec3 in_position;
// in vec3 in_color;
in vec2 in_texcoord;

// out vec3 vcolor;
// out vec2 vpos;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform mat3 view;

void main()
{
	//vpos = in_position.xy; // local coordinated before transform
	//vcolor = in_color;
	texcoord = in_texcoord;
	vec3 pos = projection * view * transform * vec3(in_position.xy, 1.0); // why not simply *in_position.xyz ?
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}