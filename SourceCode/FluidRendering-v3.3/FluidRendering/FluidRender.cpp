#include "FluidRender.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL.h>

CFluidRender::CFluidRender(void)
{
	m_WTransformMat   = glm::mat4(1.0);
	m_VTransformMat   = glm::mat4(1.0);
	m_PTransformMat   = glm::mat4(1.0);
	m_WVTransformMat  = glm::mat4(1.0);
	m_WVPTransformMat = glm::mat4(1.0);
	m_SceneWidth = WIN_WIDTH;
	m_SceneHeight = WIN_HEIGHT;

	m_ThicknessScale = 4.0;
	m_Radius = 0.05;

}

CFluidRender::~CFluidRender(void)
{
	delete m_Particle;
	delete m_pDepthShader;
}

//******************************************************************
//FUNCTION:
void CFluidRender::initializeFluidRender()
{
	__initializeParams();
	__initializeGBuffer();
	__initializePlane();
}

//******************************************************************
//FUNCTION:
void CFluidRender::initializeCubemap(const std::string* vCubemapFileNames)
{
	m_pCubemap = new CCubemap();
	_ASSERT(m_pCubemap->LoadCubeMap(vCubemapFileNames));
}

//******************************************************************
//FUNCTION:
void CFluidRender::updateFluidParticle(const SFluidParticle& vFluidParticle, int vParticleNumber)
{
	m_ParticleNumber = vParticleNumber;
	m_Particle = new SVertex[vParticleNumber];
	for (int i=0; i<vParticleNumber; i++)
	{
		m_Particle[i] = vFluidParticle.m_ParticleSet[i];
	}
	__initializeParticlesBuffer();
}

//******************************************************************
//FUNCTION:
void CFluidRender::render()
{
	__generateThickness();
	__generateDepth();
	__smoothDepth(60);
	__calculateNormal();
	__drawSkyBoxScene();
	__compositeEffects();

	__drawPlane();

}

//******************************************************************
//FUNCTION:
void CFluidRender::setCamera(const SCamera& vCamera)
{
	m_Camera = vCamera;
//	m_VTransformMat = glm::lookAt(vCamera.m_Eye, vCamera.m_Target, vCamera.m_Up);
	m_VTransformMat = glm::lookAt(vCamera.m_Eye, vCamera.m_Eye + vCamera.m_Target, vCamera.m_Up);
}

//******************************************************************
//FUNCTION:
void CFluidRender::setProjectInfo(const SProjectInfo& vProjectInfo)
{
	m_ProjectInfo = vProjectInfo;
	m_SceneWidth = vProjectInfo.m_WindosWidth;
	m_SceneHeight = vProjectInfo.m_WindosHeight;
	float aspect = m_SceneWidth / m_SceneHeight;
	m_PTransformMat = glm::perspective(vProjectInfo.m_Fov, aspect, vProjectInfo.m_Near, vProjectInfo.m_Far);
}

//******************************************************************
//FUNCTION:
void CFluidRender::__initializeTransform()
{
	m_WVPTransformMat = m_PTransformMat * m_VTransformMat * m_WTransformMat;
	m_WVTransformMat  = m_VTransformMat * m_WTransformMat;
	GLuint WVTransformLocation = m_pDepthShader->getUniformLocation("uWVTransform");
	GLuint WVPTransformLocation = m_pDepthShader->getUniformLocation("uWVPTransform");
	GLuint PTransformLocation = m_pDepthShader->getUniformLocation("uPTransform");

	glUniformMatrix4fv(WVPTransformLocation, 1, GL_FALSE, glm::value_ptr(m_WVPTransformMat));
	glUniformMatrix4fv(WVTransformLocation, 1, GL_FALSE, glm::value_ptr(m_WVTransformMat));
	glUniformMatrix4fv(PTransformLocation, 1, GL_FALSE, glm::value_ptr(m_PTransformMat));
}

//******************************************************************
//FUNCTION:
void CFluidRender::__initializeParams()
{
	m_pThicknessShader = new CShader;
	m_pThicknessShader->addShader(GL_VERTEX_SHADER, "pointV.glsl");
	m_pThicknessShader->addShader(GL_FRAGMENT_SHADER, "thicknessF.glsl");
	m_pThicknessShader->compileShader();

	m_pDepthShader = new CShader;
	m_pDepthShader->addShader(GL_VERTEX_SHADER, "pointV.glsl");
	m_pDepthShader->addShader(GL_FRAGMENT_SHADER, "depthF.glsl");
	m_pDepthShader->compileShader();

	m_pPlaneShader = new CShader;
	m_pPlaneShader->addShader(GL_VERTEX_SHADER, "planeV.glsl");
	m_pPlaneShader->addShader(GL_FRAGMENT_SHADER, "planeF.glsl");
	m_pPlaneShader->compileShader();

	m_pCurvatureFlowShader = new CShader;
	m_pCurvatureFlowShader->addShader(GL_VERTEX_SHADER, "planeV.glsl");
	m_pCurvatureFlowShader->addShader(GL_FRAGMENT_SHADER , "curvatureFlowF.glsl");
	m_pCurvatureFlowShader->compileShader();

	m_pAverageNormalShader = new CShader;
	m_pAverageNormalShader->addShader(GL_VERTEX_SHADER, "planeV.glsl");
	m_pAverageNormalShader->addShader(GL_FRAGMENT_SHADER, "averageNormalF.glsl");
	m_pAverageNormalShader->compileShader();

	m_pNormalShader = new CShader;
	m_pNormalShader->addShader(GL_VERTEX_SHADER, "planeV.glsl");
	m_pNormalShader->addShader(GL_FRAGMENT_SHADER, "normalF.glsl");
	m_pNormalShader->compileShader();

	m_pCompositeShader = new CShader;
	m_pCompositeShader->addShader(GL_VERTEX_SHADER, "planeV.glsl");
	m_pCompositeShader->addShader(GL_FRAGMENT_SHADER, "compositeF.glsl");
	m_pCompositeShader->compileShader();

	glGenBuffers(1, &m_PositionVBO);
}

//******************************************************************
//FUNCTION: 
void CFluidRender::__initializeParticlesBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_PositionVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SVertex)*m_ParticleNumber, m_Particle, GL_STATIC_DRAW);
}

//******************************************************************
//FUNCTION:
void CFluidRender::__initializeGBuffer()
{
	m_ThicknessFBO.creatFBO();
	m_ThicknessFBO.addBufferToFBO(m_ThicknessTex, 0);
	m_ThicknessFBO.bindFBO();
	GLenum ThicknessFBODrawBuffer[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, ThicknessFBODrawBuffer);
	m_ThicknessFBO.banFBO();

	m_DepthFBO.creatFBO();
	m_DepthFBO.addBufferToFBO(m_DepthTex, 0);
	m_DepthFBO.bindFBO();
	GLenum DepthFBODrawBuffer[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DepthFBODrawBuffer);
	m_DepthFBO.banFBO();

	m_SmoothFBO.creatFBO();
	m_SmoothFBO.addBufferToFBO(m_SmoothDepthTex, 0);
	m_SmoothFBO.bindFBO();
	GLenum SmoothFBODrawBuffer[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, SmoothFBODrawBuffer);
	m_SmoothFBO.banFBO();

	m_AverageNormalFBO.creatFBO();
	m_AverageNormalFBO.addBufferToFBO(m_AverageNormalTex, 0);
	m_AverageNormalFBO.bindFBO();
	GLenum AverageNormalFBODrawBuffer[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, AverageNormalFBODrawBuffer);
	m_AverageNormalFBO.banFBO();

	m_NormalFBO.creatFBO();
	m_NormalFBO.addBufferToFBO(m_NormalTex, 0);
	m_NormalFBO.bindFBO();
	GLenum NormalFBODrawBuffer[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, NormalFBODrawBuffer);
	m_NormalFBO.banFBO();

	m_CompositeFBO.creatFBO();
	m_CompositeFBO.addBufferToFBO(m_CompositeTex, 0);
	m_CompositeFBO.bindFBO();
	GLenum LightFBODrawBuffer[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, LightFBODrawBuffer);
	m_CompositeFBO.banFBO();

	m_EnvironmentFBO.creatFBO();
	m_EnvironmentFBO.addBufferToFBO(m_EnvironmentTex, 0);
	m_EnvironmentFBO.bindFBO();
	GLenum EnvironmentFBODrawBuffer[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, EnvironmentFBODrawBuffer);
	m_EnvironmentFBO.banFBO();
}

//******************************************************************
//FUNCTION:
void CFluidRender::__generateThickness()
{
	m_ThicknessFBO.bindFBO();
	m_pThicknessShader->enable();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);

	__initializeTransform();
	GLuint PointRadiusLocation = m_pDepthShader->getUniformLocation("uPointRadius");
	GLuint PointScaleLocation  = m_pDepthShader->getUniformLocation("uPointScale");
	glUniform1f(PointRadiusLocation, m_ThicknessScale*m_Radius);
	glUniform1f(PointScaleLocation, m_SceneHeight*(1.0f / (tanf(m_ProjectInfo.m_Fov * 0.5))));
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_PositionVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), 0);
	glDrawArrays(GL_POINTS, 0, m_ParticleNumber);

	glDisableVertexAttribArray(0);
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDisable(GL_DEPTH_TEST);
	m_pThicknessShader->ban();
	m_ThicknessFBO.banFBO();
}

//******************************************************************
//FUNCTION:
void CFluidRender::__generateDepth()
{
	m_DepthFBO.bindFBO();
	m_pDepthShader->enable();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	__initializeTransform();
	GLuint PointRadiusLocation = m_pDepthShader->getUniformLocation("uPointRadius");
	GLuint PointScaleLocation  = m_pDepthShader->getUniformLocation("uPointScale");
	glUniform1f(PointRadiusLocation, m_ThicknessScale*m_Radius);
	glUniform1f(PointScaleLocation, m_SceneHeight*(1.0f / (tanf(m_ProjectInfo.m_Fov * 0.5))));
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_PositionVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), 0);
	glDrawArrays(GL_POINTS, 0, m_ParticleNumber);

	glDisableVertexAttribArray(0);
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_DEPTH_TEST);
	m_pDepthShader->ban();
	m_DepthFBO.banFBO();
}

//******************************************************************
//FUNCTION:
void CFluidRender::__smoothDepth(int vIterations)
{
	__initializeSmoothParams();
	m_SmoothFBO.bindFBO();
	m_pCurvatureFlowShader->enable();
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_PlaneVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_DepthTex);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableVertexAttribArray(0);

	m_pCurvatureFlowShader->ban();
	glBindTexture(GL_TEXTURE_2D, 0);
	m_SmoothFBO.banFBO();

	for (int i=0; i<vIterations-1; i++)
	{
		GLuint TempTex;
		glGenTextures(1, &TempTex);  
		glBindTexture(GL_TEXTURE_2D, TempTex);  
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		GLuint t = TempTex;
		TempTex = m_SmoothDepthTex;
		m_SmoothDepthTex = t;
		m_SmoothFBO.bindFBO();
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SmoothDepthTex, 0);
		m_pCurvatureFlowShader->enable();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, m_PlaneVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TempTex);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableVertexAttribArray(0);

		glBindTexture(GL_TEXTURE_2D, 0);
		m_pCurvatureFlowShader->ban();
		m_SmoothFBO.banFBO();

		glDeleteTextures(1, &TempTex);
	}
	glDisable(GL_DEPTH_TEST);
}

//******************************************************************
//FUNCTION:
void CFluidRender::__initializeSmoothParams()
{
	m_pCurvatureFlowShader->enable();
	GLuint DepthSamplerLocation = m_pCurvatureFlowShader->getUniformLocation("uDepthSampler");
	GLuint ScreenSizeLocation = m_pCurvatureFlowShader->getUniformLocation("uScreenSize");
	GLuint CFThresholdLocation = m_pCurvatureFlowShader->getUniformLocation("uCFThreshold");
	GLuint CFFactoryLcoation = m_pCurvatureFlowShader->getUniformLocation("uCFFactory");
	GLuint ProjectFovLocation = m_pCurvatureFlowShader->getUniformLocation("uProjectFov");

	glUniform1i(DepthSamplerLocation, 0);
	glUniform2f(ScreenSizeLocation, m_SceneWidth, m_SceneHeight);
	glUniform1f(CFThresholdLocation, 1);
	glUniform1f(CFFactoryLcoation, 0.01);
	glUniform1f(ProjectFovLocation, m_ProjectInfo.m_Fov);

	m_pCurvatureFlowShader->ban();
}

//******************************************************************
//FUNCTION:
void CFluidRender::__calculateNormal()
{
	m_AverageNormalFBO.bindFBO();
	m_pAverageNormalShader->enable();
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLuint DepthSamplerLocation = m_pAverageNormalShader->getUniformLocation("uDepthSampler");
	GLuint ScreenSizeLocation = m_pAverageNormalShader->getUniformLocation("uScreenSize");
	GLuint ProjectFovLocation = m_pAverageNormalShader->getUniformLocation("uProjectFov");
	glUniform1i(DepthSamplerLocation, 0);
	glUniform2f(ScreenSizeLocation, m_SceneWidth, m_SceneHeight);
	glUniform1f(ProjectFovLocation, m_ProjectInfo.m_Fov);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_PlaneVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_SmoothDepthTex);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableVertexAttribArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_DEPTH_TEST);
	m_pAverageNormalShader->ban();
	m_AverageNormalFBO.banFBO();

	m_NormalFBO.bindFBO();
	m_pNormalShader->enable();
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLuint AverageNormalSamplerLocation = m_pNormalShader->getUniformLocation("uAverageNormalSampler");
	GLuint NormalDepthSamplerLocation = m_pNormalShader->getUniformLocation("uDepthSampler");
	GLuint SampleOffsetLocation = m_pNormalShader->getUniformLocation("uSampleOffset");
	GLuint NormalScreenSizeLocation = m_pNormalShader->getUniformLocation("uScreenSize");
	glUniform1i(NormalDepthSamplerLocation, 0);
	glUniform1i(AverageNormalSamplerLocation, 1);
	glUniform1i(SampleOffsetLocation, 4);
	glUniform2f(NormalScreenSizeLocation, m_SceneWidth, m_SceneHeight);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_PlaneVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_DepthTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_AverageNormalTex);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableVertexAttribArray(0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_DEPTH_TEST);
	m_pNormalShader->ban();
	m_NormalFBO.banFBO();

}

//******************************************************************
//FUNCTION:
void CFluidRender::__compositeEffects()
{
	m_CompositeFBO.bindFBO();
	m_pCompositeShader->enable();
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	__initializeCompositeEffectParams();

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_PlaneVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_AverageNormalTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_SmoothDepthTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_ThicknessTex);
	m_pCubemap->bindTexture(GL_TEXTURE3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, m_EnvironmentTex);
	
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableVertexAttribArray(0);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_DEPTH_TEST);
	m_pCompositeShader->ban();
	m_CompositeFBO.banFBO();
}

//******************************************************************
//FUNCTION:
void CFluidRender::__drawPlane()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_pPlaneShader->enable();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glClearColor(0, 0, 0, 0);
//	glClearColor(0.7, 0.7, 0.7, 1.0);
//	glClearColor(1, 1, 1, 1);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_PlaneVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_CompositeTex);
	glDrawArrays(GL_QUADS, 0, 4);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(0);
	
	glReadBuffer(GL_FRONT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	unsigned char* Data = new unsigned char[WIN_WIDTH * WIN_HEIGHT * 3];
	glReadPixels(0, 0, WIN_WIDTH, WIN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, Data);
	static unsigned int FrameNumber = 0;
	std::string FileName = "../Capture/FluidRendering_";
	FileName = FileName + std::to_string(FrameNumber) + ".bmp";
	SOIL_save_image(FileName.c_str(), SOIL_SAVE_TYPE_BMP, WIN_WIDTH, WIN_HEIGHT, 3, Data);
	FrameNumber++;

	m_pPlaneShader->ban();

	//	glGetTexImage(m_CompositeTex, 0, GL_RGB, GL_UNSIGNED_BYTE,voData);
}

//******************************************************************
//FUNCTION:
void CFluidRender::__drawSkyBoxScene()
{
	m_EnvironmentFBO.bindFBO();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_pCubemap->initializeCubemap(m_Camera, m_ProjectInfo);
	m_pCubemap->draw();
	m_EnvironmentFBO.banFBO();
}

//******************************************************************
//FUNCTION:
void CFluidRender::__initializePlane()
{
	glm::vec3 Vertices[4] = {
		glm::vec3(-1.0, -1.0, 0.0),
		glm::vec3(1.0, -1.0, 0.0),
		glm::vec3(1.0, 1.0, 0.0),
		glm::vec3(-1.0, 1.0, 0.0)
	};

	glGenBuffers(1, &m_PlaneVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_PlaneVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
	m_pPlaneShader->enable();
	GLuint ColorSamplerLocation = m_pPlaneShader->getUniformLocation("uColorSampler");
	GLuint ScreenSizeLocation = m_pPlaneShader->getUniformLocation("uScreenSize");

	glUniform1i(ColorSamplerLocation, 0);
	glUniform2f(ScreenSizeLocation, float(m_SceneWidth), float(m_SceneHeight));
	m_pPlaneShader->ban();
}

//******************************************************************
//FUNCTION:
void CFluidRender::__initializeCompositeEffectParams()
{
	GLuint EyePosLocation = m_pCompositeShader->getUniformLocation("uEyePos");
	GLuint NormalSamplerLocation = m_pCompositeShader->getUniformLocation("uNormalSampler");
	GLuint DepthSamplerLocation = m_pCompositeShader->getUniformLocation("uDepthSampler");
	GLuint ThicknessSamplerLocation = m_pCompositeShader->getUniformLocation("uThicknessSampler");
	GLuint EnvironmentSamplerLocation = m_pCompositeShader->getUniformLocation("uEnvironmentSampler");
	GLuint CubemapSamplerLocation = m_pCompositeShader->getUniformLocation("uCubemapSampler");
	GLuint ScreenSizeLocation = m_pCompositeShader->getUniformLocation("uScreenSize");
	GLuint ClipPosToEyeLocation = m_pCompositeShader->getUniformLocation("uClipPosToEye");
	GLuint WVTransformInverseLocation = m_pCompositeShader->getUniformLocation("uWVTransformInverse");

	glm::mat4 WVTransformInverse = glm::inverse(m_WVTransformMat);

	glUniform1i(NormalSamplerLocation, 0);
	glUniform1i(DepthSamplerLocation, 1);
	glUniform1i(ThicknessSamplerLocation, 2);
	glUniform1i(CubemapSamplerLocation, 3);
	glUniform1i(EnvironmentSamplerLocation, 4);
	glUniform2f(ScreenSizeLocation, m_SceneWidth, m_SceneHeight);
	glUniform3f(EyePosLocation, m_Camera.m_Eye.x, m_Camera.m_Eye.y, m_Camera.m_Eye.z);
	glUniform2f(ClipPosToEyeLocation, tanf(m_ProjectInfo.m_Fov*0.5f) * ((float)m_SceneWidth / (float)m_SceneHeight), tanf(m_ProjectInfo.m_Fov*0.5f));
	glUniformMatrix4fv(WVTransformInverseLocation, 1, GL_FALSE, glm::value_ptr(WVTransformInverse));

	SDirectionalLight DirectionalLight;
	DirectionalLight.Base.AmbientIntensity = 0.5;
	DirectionalLight.Base.Color = glm::vec3(1.0, 1.0, 1.0);
	DirectionalLight.Base.DiffuseIntensity = 0.9;
	DirectionalLight.Base.SpecularIntensity = 0.9;
	DirectionalLight.Base.SpecularPower = 32.0;
	DirectionalLight.Direction = glm::vec3(0.0, 1.0, 0.0);

// 	SPointLight PointLight;
// 	PointLight.Base.AmbientIntensity = 0.1;
// 	PointLight.Base.Color = glm::vec3(1.0, 1.0, 1.0);
// 	PointLight.Base.DiffuseIntensity = 0.9;
// 	PointLight.Base.SpecularIntensity = 1.0;
// 	PointLight.Base.SpecularPower = 32.0;
// 	PointLight.Position = glm::vec3(0.0, 10.0, -5.0);
// 	PointLight.Atten.Constant = 1.0;
// 	PointLight.Atten.Linear   = 0.1;
// 	PointLight.Atten.Exp      = 0.0;

	GLuint DirectionalLightBaseAmbientIntensityLocation = m_pCompositeShader->getUniformLocation("uDirectionalLight.Base.AmbientIntensity");
	GLuint DirectionalLightBaseDiffuseIntensityLocation = m_pCompositeShader->getUniformLocation("uDirectionalLight.Base.DiffuseIntensity");
	GLuint DirectionalLightBaseColorLocation = m_pCompositeShader->getUniformLocation("uDirectionalLight.Base.Color");
	GLuint DirectionalLightBaseSpecularIntensityLocation = m_pCompositeShader->getUniformLocation("uDirectionalLight.Base.SpecularIntensity");
	GLuint DirectionalLightBaseSpecularPowerLocation = m_pCompositeShader->getUniformLocation("uDirectionalLight.Base.SpecularPower");
	GLuint DirectionalLightDirectionLocation = m_pCompositeShader->getUniformLocation("uDirectionalLight.Direction");

	glUniform1f(DirectionalLightBaseAmbientIntensityLocation, DirectionalLight.Base.AmbientIntensity);
	glUniform3f(DirectionalLightBaseColorLocation, DirectionalLight.Base.Color.x, DirectionalLight.Base.Color.y, DirectionalLight.Base.Color.z);
	glUniform1f(DirectionalLightBaseDiffuseIntensityLocation, DirectionalLight.Base.DiffuseIntensity);
	glUniform1f(DirectionalLightBaseSpecularIntensityLocation, DirectionalLight.Base.SpecularIntensity);
	glUniform1f(DirectionalLightBaseSpecularPowerLocation, DirectionalLight.Base.SpecularPower);
	glUniform3f(DirectionalLightDirectionLocation, DirectionalLight.Direction.x, DirectionalLight.Direction.y, DirectionalLight.Direction.z);

// 	GLuint PointLightBaseAmbientIntensityLocation  = m_pCompositeShader->getUniformLocation("uPointLight.Base.AmbientIntensity");
// 	GLuint PointLightBaseDiffuseIntensityLocation  = m_pCompositeShader->getUniformLocation("uPointLight.Base.DiffuseIntensity");
// 	GLuint PointLightBaseColorLocation	           = m_pCompositeShader->getUniformLocation("uPointLight.Base.Color");
// 	GLuint PointLightBaseSpecularIntensityLocation = m_pCompositeShader->getUniformLocation("uPointLight.Base.SpecularIntensity");
// 	GLuint PointLightBaseSpecularPowerLocation     = m_pCompositeShader->getUniformLocation("uPointLight.Base.SpecularPower");
// 	GLuint PointLightPositionLocation			   = m_pCompositeShader->getUniformLocation("uPointLight.Position");
// 	GLuint PointLightAttenConstantLocation		   = m_pCompositeShader->getUniformLocation("uPointLight.Atten.Constant");
// 	GLuint PointLightAttenLinearLocation		   = m_pCompositeShader->getUniformLocation("uPointLight.Atten.Linear");
// 	GLuint PointLightAttenExpLocation			   = m_pCompositeShader->getUniformLocation("uPointLight.Atten.Exp");
// 
// 	glUniform1f(PointLightBaseAmbientIntensityLocation, PointLight.Base.AmbientIntensity);
// 	glUniform1f(PointLightBaseDiffuseIntensityLocation, PointLight.Base.DiffuseIntensity);
// 	glUniform3f(PointLightBaseColorLocation, PointLight.Base.Color.x, PointLight.Base.Color.y, PointLight.Base.Color.z);
// 	glUniform1f(PointLightBaseSpecularIntensityLocation, PointLight.Base.SpecularIntensity);
// 	glUniform1f(PointLightBaseSpecularPowerLocation, PointLight.Base.SpecularPower);
// 	glUniform3f(PointLightPositionLocation, PointLight.Position.x, PointLight.Position.y, PointLight.Position.z);
// 	glUniform1f(PointLightAttenConstantLocation, PointLight.Atten.Constant);
// 	glUniform1f(PointLightAttenLinearLocation, PointLight.Atten.Linear);
// 	glUniform1f(PointLightAttenExpLocation, PointLight.Atten.Exp);
}