#version 430
layout (location = 0) out vec4 FragNormal;

uniform sampler2D gDepthSampler;
uniform vec2 gScreenSize;
uniform vec2 uClipPosToEye;
uniform float uMaxDepth;
uniform float uMinDepth;


vec3 viewportToEyeSpace(vec2 coord, float eyeZ)
{
	// find position at z=1 plane
	vec2 uv = (coord*2.0 - vec2(1.0))*uClipPosToEye;

	return vec3(-uv*eyeZ, eyeZ);
}

float calculateDepth(float vValue)
{
	float Depth = uMaxDepth - (vValue * (uMaxDepth - uMinDepth));
	return Depth;
}

vec2 calculateTexCoord()
{
    return gl_FragCoord.xy / gScreenSize;
}

void main()
{
	vec2 TexCoord = calculateTexCoord();
	float texDepth = texture(gDepthSampler, TexCoord).x;
	if (texDepth <= 0) discard;
	float Depth = calculateDepth(texDepth);
	vec3 EyePos = viewportToEyeSpace(TexCoord, Depth);

	float OffsetX = 1 / gScreenSize.x;
	float OffsetY = 1 / gScreenSize.y;

	vec2 Left = TexCoord + vec2(-OffsetX, 0.0);
	float LeftDepth = calculateDepth(texture(gDepthSampler, Left).x);
	vec3 zl = EyePos - viewportToEyeSpace(Left, LeftDepth);

	vec2 Right = TexCoord + vec2(OffsetX, 0.0);
	float RightDepth = calculateDepth(texture(gDepthSampler, Right).x);
	vec3 zr = viewportToEyeSpace(Right, RightDepth) - EyePos;

	vec2 Bottom = TexCoord + vec2(0.0, -OffsetY);
	float BottomDepth = calculateDepth(texture(gDepthSampler, Bottom).x);
	vec3 zb = EyePos - viewportToEyeSpace(Bottom, BottomDepth);

	vec2 Top = TexCoord + vec2(0.0, OffsetY);
	float TopDepth = calculateDepth(texture(gDepthSampler, Top).x);
	vec3 zt = viewportToEyeSpace(Top, TopDepth) - EyePos;

	vec3 dx = zl.z < zr.z ? zl : zr;
	vec3 dy = zb.z < zt.z ? zb : zt;

	vec3 Normal = cross(dx, dy);
	Normal = normalize(Normal);
	vec3 TexNormal = Normal * 0.5 + vec3(0.5);

	FragNormal = vec4(TexNormal, 1.0);
}