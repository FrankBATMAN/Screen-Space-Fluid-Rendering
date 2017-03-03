#version 430

layout (location = 0) out vec4 FragNormal;

uniform sampler2D uDepthSampler;
uniform vec2 uScreenSize;
uniform float uProjectFov;

vec2 calculateTexCoord()
{
    return gl_FragCoord.xy / uScreenSize;
}

float dz2x(float x, float y)
{
	if(x<=0 || y<=0 || x>=1 || y>=1)
	{
		return 0;
	}

	float v0 = textureOffset(uDepthSampler, vec2(x, y), ivec2(-1, 0)).x;
	float v1 = texture(uDepthSampler, vec2(x, y) ).x;
	float v2 = textureOffset(uDepthSampler, vec2(x, y), ivec2(1, 0)).x;

	float ret = 0;

	if( (v0 == 0) && (v2 != 0))
	{
		ret = (v2 - v1);
	}
	else if((v2 ==0) && (v0 != 0))
	{
		ret = (v1 - v0);
	}
	else 
	{
		ret = (v2 - v0) / 2.0f;
	}

	return ret;
}

float dz2y(float x, float y)
{
	if( x<=0 || y<=0 || x>=1 || y>=1)
	{
		return 0;
	}

	float v0 = textureOffset(uDepthSampler, vec2(x, y), ivec2(0, -1)).x;
	float v1 = texture(uDepthSampler, vec2(x, y)).x;
	float v2 = textureOffset(uDepthSampler, vec2(x, y), ivec2(0, 1)).x;

	float ret = 0;

	if( (v0 == 0 && v2 != 0) )
	{
		ret = (v2 - v1);
	}
	else if( (v2 == 0 && v0 != 0) )
	{
		ret = (v1 - v0);
	}
	else
	{
		ret = ( v2 - v0) / 2.f;
	}

	return ret;
}

void main()
{
	vec2 TexCoord = calculateTexCoord();
	float Depth = texture(uDepthSampler, TexCoord).x;
	vec3 Normal = vec3(0.0, 0.0, 0.0);

	float OffsetX = 1 / uScreenSize.x;
	float OffsetY = 1 / uScreenSize.y;

	int SampleOffset = 6;
	int Counter = 0;

	if (Depth > 0)
	{
		for (int i=-SampleOffset; i<=SampleOffset; i++)
			for (int k=-SampleOffset; k<=SampleOffset; k++)
			{
				float fd = textureOffset(uDepthSampler, TexCoord, ivec2(i, k)).x;
				float fx = gl_FragCoord.x + i;
				float fy = gl_FragCoord.y + k;
				if ((fx>=0) && (fx<=uScreenSize.x) && (fy>=0) && (fy<=uScreenSize.y) && (fd>0))
				{
					float dz_x = dz2x(TexCoord.x+i*OffsetX, TexCoord.y+k*OffsetY);
					float dz_y = dz2y(TexCoord.x+i*OffsetX, TexCoord.y+k*OffsetY);

//					float Cx = fx==0 ? 0 : 2 / (fx * uScreenSize.x);
//					float Cy = fy==0 ? 0 : 2 / (fy * uScreenSize.y);

					float Cx = fx==0 ? 0 : 2 / (fx * tan(uProjectFov /2));
					float Cy = fy==0 ? 0 : 2 / (fy * tan(uProjectFov /2));
//					float Cy = fy==0 ? 0 : 2 / (fy * ( 1 / tan(uProjectFov / 2 ))); 

					float D = Cy * Cy * dz_x * dz_x + Cx * Cx * dz_y * dz_y + Cx * Cx * Cy * Cy * fd * fd;
					if (D == 0) continue;

					float inv_sqrtD = 1 / sqrt(D);

					Normal.x += Cy * dz_x * inv_sqrtD;
					Normal.y += Cx * dz_y * inv_sqrtD;
					Normal.z += Cx * Cy * fd * inv_sqrtD;
					Counter++;
				}
			}
		Normal = Normal / float(Counter);
		Normal = normalize(Normal);
		Normal = Normal*0.5 + vec3(0.5);
	}
	FragNormal = vec4(Normal, 1.0);
}