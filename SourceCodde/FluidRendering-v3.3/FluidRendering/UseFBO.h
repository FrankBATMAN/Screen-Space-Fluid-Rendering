#pragma once
#include<iostream>
#include<gl/glew.h>
#pragma comment(lib, "glew32.lib")

const int   TEXTURE_WIDTH   = 1280;  // NOTE: texture size cannot be larger than
const int   TEXTURE_HEIGHT  = 720;  // the rendering window size in non-FBO mode

class CFBO
{
public:
	CFBO(){}
	~CFBO(){}
	void creatFBO();
	void bindFBO();
	void banFBO();
	void addBufferToFBO(GLuint &bufferId, unsigned int n);
private:
	bool checkFramebufferStatus();
	GLuint m_fboId;
	GLuint m_depId;
};

