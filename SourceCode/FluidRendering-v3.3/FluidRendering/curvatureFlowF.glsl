#version 430

layout(location = 0) out vec4 FragColor;

uniform sampler2D uDepthSampler;
uniform vec2 uScreenSize;
uniform float uCFThreshold;
uniform float uCFFactory;
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

	if(v1 == 0)
	{
		return ret;
	}

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

	if(v1 == 0)
	{
		return ret;
	}

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

void curvatureFlow()
{
	vec2 TexCoord = calculateTexCoord();
	float Depth = texture(uDepthSampler, TexCoord).x;
	vec2 ViewSpaceCoord = gl_FragCoord.xy;

	float OffsetX = 1 / uScreenSize.x;
	float OffsetY = 1 / uScreenSize.y;

	if (Depth > 0)
	{
		float dz_x = dz2x(TexCoord.x, TexCoord.y);
		float dz_x0 = dz2x(TexCoord.x - OffsetX, TexCoord.y);
		float dz_x2 = dz2x(TexCoord.x + OffsetX, TexCoord.y);
		float dz2x2 = (dz_x2 - dz_x0) / 2.0f;

		float dz_y = dz2y(TexCoord.x, TexCoord.y);
		float dz_y0 = dz2y(TexCoord.x, TexCoord.y - OffsetY);
		float dz_y2 = dz2y(TexCoord.x, TexCoord.y + OffsetY);
		float dz2y2 = (dz_y2 - dz_y0) / 2.0f;

//		float Cx = ViewSpaceCoord.x == 0 ? 0 : 2.0 / (ViewSpaceCoord.x * uScreenSize.x);
//		float Cy = ViewSpaceCoord.y == 0 ? 0 : 2.0 / (ViewSpaceCoord.y * uScreenSize.y);

		float Cx = ViewSpaceCoord.x==0 ? 0 : 2 / (ViewSpaceCoord.x * tan(uProjectFov /2));
		float Cy = ViewSpaceCoord.y==0 ? 0 : 2 / (ViewSpaceCoord.y * tan(uProjectFov /2));
//		float Cy = ViewSpaceCoord.y==0 ? 0 : 2 / (ViewSpaceCoord.y * ( 1 / tan(uProjectFov / 2 ))); 
		
		float D = Cy * Cy * dz_x * dz_x + Cx * Cx * dz_y * dz_y + Cx * Cx * Cy * Cy * Depth * Depth;

//		if (D == 0) discard;

		float inv_D32 = 1.0 / pow(D, 1.5);
		 
		float kx = 4.0 / (uScreenSize.x * uScreenSize.x);
		float ky = 4.0 / (uScreenSize.y * uScreenSize.y);
		float dD_x = ky * pow(ViewSpaceCoord.y, -2.0) * 2 * dz_x * dz2x2  +
					 kx * dz_y * dz_y * (-2) * pow(ViewSpaceCoord.x, -3.0) +
					 2 * ky * pow(ViewSpaceCoord.y, -2.0) * kx * (-1 * pow(ViewSpaceCoord.x, -3) * Depth * Depth + pow(ViewSpaceCoord.x, -2) * Depth * dz_x);

		float dD_y = ky * (-2) * pow(ViewSpaceCoord.y, -3) * dz_x * dz_x +
					 kx * pow(ViewSpaceCoord.x, -2) * 2 * dz_y * dz2y2 +
					 2 * kx * ky * pow(ViewSpaceCoord.x, -2) * (-1 * pow(ViewSpaceCoord.y, -3) * Depth * Depth + pow(ViewSpaceCoord.y, -2) * Depth * dz_y);	
					 
		float Ex = 0.5 * dz_x * dD_x - dz2x2 * D;
		float Ey = 0.5 * dz_y * dD_y - dz2y2 * D;

		float fCF = (Cy * Ex + Cx * Ey) * inv_D32 / 2;

		fCF = fCF > uCFThreshold ? uCFThreshold : fCF;
		fCF = fCF < -uCFThreshold ? -uCFThreshold : fCF;
		Depth = Depth - fCF * uCFFactory;
		if (Depth <= 0) Depth = 0.01;
	}

	FragColor = vec4(Depth, 0.0, 0.0, 1.0);
}

void main()
{
	curvatureFlow();
}