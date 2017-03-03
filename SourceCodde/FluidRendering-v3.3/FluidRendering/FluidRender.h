#pragma once
#include "datastruct.h"
#include "ParticlesData.h"
#include "Shader.h"
#include "UseFBO.h"
#include "Cubemap.h"

class CFluidRender
{
public:

	CFluidRender(void);
	~CFluidRender(void);

	void initializeFluidRender();
	void initializeCubemap(const std::string* vCubemapFileNames);
	void setCamera(const SCamera& vCamera);
	void setProjectInfo(const SProjectInfo& vProjectInfo);

	void render();
	void updateFluidParticle(const SFluidParticle& vFluidParticle, int vParticleNumber);

private:
	int m_SceneWidth;
	int m_SceneHeight;

	int m_ParticleNumber;
	SVertex* m_Particle;
	float m_ThicknessScale;
	float m_Radius;

	CCubemap *m_pCubemap;
	SCamera m_Camera;
	SProjectInfo m_ProjectInfo;

	CFBO m_ThicknessFBO;
	CFBO m_DepthFBO;
	CFBO m_SmoothFBO;
	CFBO m_AverageNormalFBO;
	CFBO m_NormalFBO;
	CFBO m_CompositeFBO;
	CFBO m_EnvironmentFBO;

	GLuint m_PositionVBO;
	GLuint m_PlaneVBO;

	CShader* m_pThicknessShader;
	CShader* m_pDepthShader;
	CShader* m_pCurvatureFlowShader;
	CShader* m_pAverageNormalShader;
	CShader* m_pNormalShader;
	CShader* m_pCompositeShader;
	CShader* m_pPlaneShader;

	GLuint m_ThicknessTex;
	GLuint m_DepthTex;
	GLuint m_SmoothDepthTex;
	GLuint m_AverageNormalTex;
	GLuint m_NormalTex;
	GLuint m_CompositeTex;
	GLuint m_EnvironmentTex;

	glm::mat4 m_WTransformMat;
	glm::mat4 m_VTransformMat;
	glm::mat4 m_PTransformMat;
	glm::mat4 m_WVTransformMat;
	glm::mat4 m_WVPTransformMat;

	void __initializeParams();
	void __initializeParticlesBuffer();
	void __initializeGBuffer();
	void __initializeTransform();
	void __initializePlane();

	void __initializeSmoothParams();
	void __initializeCompositeEffectParams();

	void __generateThickness();
	void __generateDepth();
	void __smoothDepth(int vIterations);
	void __calculateNormal();
	void __compositeEffects();
	void __drawPlane();
	void __drawSkyBoxScene();
};

