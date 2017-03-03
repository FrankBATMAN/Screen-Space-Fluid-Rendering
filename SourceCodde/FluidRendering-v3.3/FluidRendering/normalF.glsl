#version 430

layout (location = 0) out vec4 FragNormal;

uniform sampler2D uAverageNormalSampler;
uniform sampler2D uDepthSampler;
uniform int uSampleOffset;
uniform vec2 uScreenSize;

vec3 transformNormalBack(vec3 vNormal)
{
	vec3 retNormal = (vNormal - vec3(0.5))*2;
	return retNormal;
}

vec2 calculateTexCoord()
{
    return gl_FragCoord.xy / uScreenSize;
}

void main()
{
	vec2 TexCoord = calculateTexCoord();
	float x = gl_FragCoord.x;
	float y = gl_FragCoord.y;
	float fd = texture(uDepthSampler, TexCoord).x;
	vec3 Normal = vec3(0.0, 0.0, 0.0);

	if ((int(x) % uSampleOffset == 0) && (int(y) % uSampleOffset == 0))
	{
		Normal = texture(uAverageNormalSampler, TexCoord).xyz;
	}
	else
	{
		if(fd > 0)
		{
			int GridX = int(floor(x / uSampleOffset) * uSampleOffset);
			int GridY = int(floor(y / uSampleOffset) * uSampleOffset);

			float DistPercentX = (x - GridX) * 1.f / uSampleOffset;
			float DistPercentY = (y - GridY) * 1.f / uSampleOffset;

			vec3 SampleNormal[4];
			SampleNormal[0] = vec3(0, 0, 0);
			SampleNormal[1] = vec3(0, 0, 0);
			SampleNormal[2] = vec3(0, 0, 0);
			SampleNormal[3] = vec3(0, 0, 0);

			vec2 TexCoord0 = vec2(GridX, GridY) / uScreenSize;
			vec3 tNormal = texture(uAverageNormalSampler, TexCoord0).xyz;
			SampleNormal[0] = transformNormalBack(tNormal);

			if ((x+1 < uScreenSize.x) && (GridX+uSampleOffset < uScreenSize.x))
			{
				vec2 TexCoord1 = vec2(GridX+uSampleOffset, GridY) / uScreenSize;
				tNormal = texture(uAverageNormalSampler, TexCoord1).xyz;
				SampleNormal[1] = transformNormalBack(tNormal);
			}

			if ((y+1 < uScreenSize.y) && (GridY+uSampleOffset < uScreenSize.y))
			{
				vec2 TexCoord2 = vec2(GridX, GridY+uSampleOffset) / uScreenSize;
				tNormal = texture(uAverageNormalSampler, TexCoord2).xyz;
				SampleNormal[2] = transformNormalBack(tNormal);
			}

			if ((x+1 < uScreenSize.x) && (y+1 < uScreenSize.y) && (GridX+uSampleOffset < uScreenSize.x) && (GridY+uSampleOffset < uScreenSize.y))
			{
				vec2 TexCoord3 = vec2(GridX+uSampleOffset, GridY+uSampleOffset) / uScreenSize;
				tNormal = texture(uAverageNormalSampler, TexCoord3).xyz;
				SampleNormal[3] = transformNormalBack(tNormal);
			}

			vec3 Normal01 = SampleNormal[0] * (1 - DistPercentX) + SampleNormal[1] * DistPercentX;
			vec3 Normal23 = SampleNormal[2] * (1 - DistPercentX) + SampleNormal[3] * DistPercentY;
			Normal = Normal01 * (1 - DistPercentY) + Normal23 * DistPercentY;
			Normal = normalize(Normal);
			Normal = Normal*0.5 + vec3(0.5);
		}
	}

//	Normal = texture(uAverageNormalSampler, TexCoord).xyz;
	FragNormal = vec4(Normal, 1.0);
}