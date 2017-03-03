#version 430

layout(location = 0) out vec4 FragColor;

struct SBaseLight
{
	vec3 Color;
	float AmbientIntensity;
	float DiffuseIntensity;
	float SpecularIntensity;
	float SpecularPower;
};

struct SDirectionalLight
{
	SBaseLight Base;
	vec3 Direction;
};

struct SAttenuation
{
	float Constant;
	float Linear;
	float Exp;
};

struct SPointLight
{
	SBaseLight Base;
	vec3 Position;
	SAttenuation Atten;
};

uniform SDirectionalLight uDirectionalLight;
uniform SPointLight       uPointLight;

uniform vec2 uScreenSize;
uniform sampler2D uNormalSampler;
uniform sampler2D uDepthSampler;
uniform sampler2D uThicknessSampler;
uniform sampler2D uEnvironmentSampler;
uniform samplerCube uCubemapSampler;
uniform vec2 uClipPosToEye;
uniform mat4 uWVTransformInverse;
uniform vec3 uEyePos;

vec3 transformNormalBack(vec3 vNormal)
{
	vec3 retNormal = (vNormal - vec3(0.5))*2;
	return retNormal;
}

vec3 viewportToEyeSpace(vec2 vCoord, float vEyeZ)
{
	// find position at z=1 plane
	vec2 UV = (vCoord*2.0 - vec2(1.0))* uClipPosToEye;

	return vec3(-UV * vEyeZ, vEyeZ);
}

vec2 calculateTexCoord()
{
    return gl_FragCoord.xy / uScreenSize;
}

vec4 calculateLightInternal(SBaseLight vLight, vec3 vLightDirection, vec3 vNormal, vec3 vWorldPos)
{
	vec4 AmbientColor = vec4(vLight.Color * vLight.AmbientIntensity, 1.0);
	float DiffuseFactory = dot(vNormal, -vLightDirection);

	vec4 DiffuseColor = vec4(0, 0, 0, 0);

	if (DiffuseFactory > 0)
	{
		DiffuseColor = vec4(vLight.Color * vLight.DiffuseIntensity * DiffuseFactory, 1.0);

	}

	return (AmbientColor + DiffuseColor);
}

vec4 calculateDirectionalLight(SDirectionalLight vDirectionalLight, vec3 vNormal, vec3 vWorldPos)
{
	return calculateLightInternal(vDirectionalLight.Base, vDirectionalLight.Direction, vNormal, vWorldPos);
}

vec4 calculatePointLight(SPointLight vPointLight, vec3 vNormal, vec3 vWorldPos)
{
	vec3 LightDirection = vWorldPos - vPointLight.Position;
	float Distance = length(LightDirection);
	LightDirection = normalize(LightDirection);

	vec4 Color = calculateLightInternal(vPointLight.Base, LightDirection, vNormal, vWorldPos);
	float Attenuation = vPointLight.Atten.Constant +
						vPointLight.Atten.Linear * Distance +
						vPointLight.Atten.Exp * Distance * Distance;

	return (Color / Attenuation);
}

void main()
{
	vec2 TexCoord = calculateTexCoord();
	vec3 tNormal = texture(uNormalSampler, TexCoord).xyz;
	vec3 Normal = normalize(transformNormalBack(tNormal));
	float Thickness = texture(uThicknessSampler, TexCoord).x;
	float ViewDepth = texture(uDepthSampler, TexCoord).x;

	vec3 ViewPos = viewportToEyeSpace(TexCoord, -ViewDepth);
	vec3 WorldPos = (uWVTransformInverse * vec4(ViewPos, 1.0)).xyz;

	vec4 Color = texture(uEnvironmentSampler, TexCoord);

	if (ViewDepth > 0)
	{
		vec4 FluidColor = vec4(0.1, 0.4, 0.8, 1.0);

		float Thickness = max(texture2D(uThicknessSampler, TexCoord).x, 0.4);

		float Fresnel = 0.1 + (1.0 - 0.1)* pow((1.0-max(dot(Normal,-normalize(ViewPos) ), 0.0)), 5.0);
		
		vec4 DiffuseLight = calculateDirectionalLight(uDirectionalLight, Normal, WorldPos);

		vec4 SpecularColor = vec4(0, 0, 0, 0);
		vec3 VertexToEye = normalize(uEyePos - WorldPos);
		vec3 LightReflect = normalize(reflect(uDirectionalLight.Direction, Normal));
		float SpecularFactory = dot(VertexToEye, LightReflect);
		if (SpecularFactory > 0)
		{
			SpecularFactory = pow(SpecularFactory, uDirectionalLight.Base.SpecularPower);
			SpecularColor   = vec4(uDirectionalLight.Base.Color * uDirectionalLight.Base.SpecularIntensity * SpecularFactory, 1.0);
		}

		vec3 ViewDirection = normalize(WorldPos - uEyePos);
		vec3 Reflection = reflect(ViewDirection, Normal);
		vec3 ReflectionTex = texture(uCubemapSampler, Reflection).xyz;
		vec3 ReflectionColor = mix(vec3(0.0), ReflectionTex, smoothstep(0.05, 0.3, WorldPos.y));

		float Radio = 1.0 / 1.33;
		vec3 Transmission = (1.0-(1.0-FluidColor.xyz)*Thickness*0.1);
		vec3 Refraction = refract(ViewDirection, Normal, Radio);
		vec3 RefractionColor = texture(uCubemapSampler, Refraction).xyz * Transmission;

		float LightAttenuation = dot(uDirectionalLight.Direction, Normal) * 0.05;
		DiffuseLight = vec4(mix(DiffuseLight.xyz, vec3(1.0), (LightAttenuation*0.5 + 0.5)*0.4), 1.0);
		Color = FluidColor * DiffuseLight + (vec4(mix(RefractionColor, ReflectionColor, Fresnel), 1.0) + SpecularColor);
	}


	FragColor = vec4(Color.xyz, 1.0);
}