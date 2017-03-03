#pragma once
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <glm/glm.hpp>
#include <string>
#include "datastruct.h" 

class CShader;

class CCubemap
{
public:
	CCubemap(void);
	~CCubemap(void);
	bool LoadCubeMap(const std::string* vFileNames);
	void initializeCubemap(const SCamera& vCamera, const SProjectInfo& vProjection);
	void draw();
	void bindTexture(GLenum vTexUnit);

private:
	GLuint m_CubemapTex;
	CShader *m_pShader;
	GLuint m_VBO;
	GLuint m_IBO;
	bool __loadTGA(const std::string& vFileName, GLubyte* &voImageData, GLuint& voWidth, GLuint& voHeight, GLenum& voType);

	void __initCubemap();
};

