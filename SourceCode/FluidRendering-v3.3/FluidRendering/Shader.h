#pragma once
#include <gl/glew.h>
#pragma comment(lib, "glew32.lib")
#include <gl/freeglut.h>

#include <string>
#include <list>

class CShader
{
public:
	CShader(void);
	~CShader(void);

	bool addShader(GLenum vShaderType, const std::string& vShaderFileName);
	bool compileShader();
	GLuint getUniformLocation(const std::string& vUniformName);
	void enable();
	void ban();

private:
	GLuint m_ShaderProgram;
	std::list<GLuint> m_ShaderObjectList;

	bool __readShaderFile(const std::string& vShaderFileName, std::string& voShaderData);
};

