#version 430

out vec4 FragColor;

uniform sampler2D uDepthSampler;
uniform sampler2D uColorSampler;
uniform vec2 uScreenSize;

vec2 calculateTexCoord()
{
    return gl_FragCoord.xy / uScreenSize;
}

void main()
{
	vec2 TexCoord = calculateTexCoord();
	float ViewDepth = texture(uDepthSampler, TexCoord).x;
	vec3 Color    = texture(uColorSampler, TexCoord).xyz;                              
	FragColor = vec4(Color, 1.0);
}