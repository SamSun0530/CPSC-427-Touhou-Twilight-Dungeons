#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float time;
uniform float strength;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
    vec2 offset = texcoord;
    offset.x += sin(time*strength*2.5 + texcoord.y * 2.0) * 0.005;
    offset.y += cos(time*strength*2.5 + texcoord.x * 2.0) * 0.005;
    vec4 texColor = texture(sampler0, offset);
    float gradient = texcoord.x + texcoord.y - (sin(time*strength) + 1.0);
    
    float gradientInverse = -(texcoord.x + texcoord.y - (sin(time*strength) + 1.0));
    texColor.rgb +=1.5* texColor.rgb * smoothstep(-0.2, 0.2, gradient) * smoothstep(-0.2, 0.2, gradientInverse);

    // Sample the texture with constrained coordinates
    color = vec4(fcolor, 1.0) * texColor;
}
