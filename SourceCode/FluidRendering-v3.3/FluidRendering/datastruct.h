#pragma once
#include <glm/glm.hpp>

#ifndef WIN_WIDTH
#define WIN_WIDTH 1280
#endif // !WIN_WIDTH

#ifndef WIN_HEIGHT
#define WIN_HEIGHT 720
#endif

struct SVertex
{
	glm::vec3 m_Pos;
	glm::vec2 m_Tex;
	glm::vec3 m_Normal;

	SVertex() {}

	SVertex(float vX, float vY, float vZ)
	{
		m_Pos.x = vX;
		m_Pos.y = vY;
		m_Pos.z = vZ;

		m_Tex = glm::vec2(0.0, 0.0);
		m_Normal = glm::vec3(0.0, 0.0, 0.0);
	}

	SVertex(glm::vec3 vPos, glm::vec2 vTex)
	{
		m_Pos = vPos;
		m_Tex = vTex;
		m_Normal = glm::vec3(0.0, 0.0, 0.0);
	}

	SVertex(glm::vec3 vPos, glm::vec2 vTex, glm::vec3 vNormal)
	{
		m_Pos = vPos;
		m_Tex = vTex;
		m_Normal = vNormal;
	}
};

struct SCamera
{
	glm::vec3 m_Eye;
	glm::vec3 m_Target;
	glm::vec3 m_Up;

	SCamera() {}

	SCamera(glm::vec3 vEye, glm::vec3 vTarget, glm::vec3 vUp)
	{
		m_Eye = vEye;
		m_Target = vTarget;
		m_Up = vUp;
	}
};

struct SProjectInfo
{
	float m_Fov;
	float m_WindosWidth;
	float m_WindosHeight;
	float m_Near;
	float m_Far;

	SProjectInfo() {}

	SProjectInfo(float vFOV, float vWinWidth, float vWinHeight, float vNear, float vFar)
	{
		m_Fov = vFOV;
		m_WindosWidth = vWinWidth;
		m_WindosHeight = vWinHeight;
		m_Near = vNear;
		m_Far = vFar;
	}
};

struct SBaseLight
{
	glm::vec3 Color;
	float AmbientIntensity;
	float DiffuseIntensity;
	float SpecularIntensity;
	float SpecularPower;
};

struct SDirectionalLight
{
	SBaseLight Base;
	glm::vec3 Direction;
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
	glm::vec3 Position;
	SAttenuation Atten;
};