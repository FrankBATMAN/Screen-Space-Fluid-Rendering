#version 430

layout(location = 0) out vec4 FragColor;

in vec3 vsWorldPos;

uniform float uPointRadius;
uniform mat4 uPTransform;

void main()
{
	vec3 Normal;
	Normal.xy = gl_PointCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
	float mag = dot(Normal.xy, Normal.xy);
	if (mag > 1.0) discard;
	Normal.z = sqrt(1.0 - mag);

	FragColor = vec4(Normal.z * 0.005);

	vec3 EyePos = vsWorldPos + Normal * uPointRadius * 0.8;
	vec4 NDCPos = uPTransform * vec4(EyePos, 1.0);
	NDCPos.z = NDCPos.z / NDCPos.w;
	gl_FragDepth = NDCPos.z * 0.5 + 0.5;
}