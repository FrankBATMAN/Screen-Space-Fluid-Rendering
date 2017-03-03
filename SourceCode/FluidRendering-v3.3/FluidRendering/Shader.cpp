#include "Shader.h"
#include <fstream>

#define INVALID_UNIFORM_LOCATION 0xffffffff

CShader::CShader(void)
{
	m_ShaderProgram = glCreateProgram();
}


CShader::~CShader(void)
{
}

//******************************************************************
//FUNCTION:
bool CShader::addShader(GLenum vShaderType, const std::string& vShaderFileName)
{
	std::string ShaderData;
	if (!__readShaderFile(vShaderFileName, ShaderData))
	{
		return false;
	}
	GLuint ShaderObject = glCreateShader(vShaderType);

	const GLchar *pShaderData = ShaderData.c_str();
	const GLint Length = ShaderData.length();
	glShaderSource(ShaderObject, 1, &pShaderData, &Length);
	glCompileShader(ShaderObject);

	GLint Success;
	GLchar ErrorLog[1024];
	glGetShaderiv(ShaderObject, GL_COMPILE_STATUS, &Success);
	if (!Success)
	{
		glGetShaderInfoLog(ShaderObject, 1024, NULL, ErrorLog);
		fprintf(stderr, "Error compiling shader '%s' : '%s' \n",vShaderFileName.c_str(), ErrorLog);
		return false;
	}

	m_ShaderObjectList.push_back(ShaderObject);
	glAttachShader(m_ShaderProgram, ShaderObject);
	return true;
}

//******************************************************************
//FUNCTION:
bool CShader::compileShader()
{
	GLint Success = 0;
	GLchar ErrorLog[1024] = {0};

	glLinkProgram(m_ShaderProgram);
	glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &Success);
	if (!Success)
	{
		glGetProgramInfoLog(m_ShaderProgram, 1024, NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program : '%s' \n", ErrorLog);
		return false;
	}

	glValidateProgram(m_ShaderProgram);
	glGetProgramiv(m_ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success)
	{
		glGetProgramInfoLog(m_ShaderProgram, 1024, NULL, ERROR);
		fprintf(stderr, "Error validating shader program : '%s' \n", ErrorLog);
		return false;
	}

	for (std::list<GLuint>::iterator it = m_ShaderObjectList.begin(); it!=m_ShaderObjectList.end(); it++)
	{
		glDeleteShader(*it);
	}
	m_ShaderObjectList.clear();

	return Success;
}

//******************************************************************
//FUNCTION:
GLuint CShader::getUniformLocation(const std::string& vUniformName)
{
	GLuint Location = glGetUniformLocation(m_ShaderProgram, vUniformName.c_str());

	if (Location == INVALID_UNIFORM_LOCATION)
	{
		fprintf(stderr,"Warning : Unable to get the location of uniform '%s' \n", vUniformName.c_str());
	}
	return Location;
}

//******************************************************************
//FUNCTION:
void CShader::enable()
{
	glUseProgram(m_ShaderProgram);
}

//******************************************************************
//FUNCTION:
void CShader::ban()
{
	glUseProgram(0);
}

//******************************************************************
//FUNCTION:
bool CShader::__readShaderFile(const std::string& vShaderFileName, std::string& voShaderData)
{
	bool Success = false;
	std::ifstream InFile(vShaderFileName.c_str());

	if (InFile.is_open())
	{
		std::string LineStr;
		while (getline(InFile, LineStr))
		{
			voShaderData.append(LineStr.c_str());
			voShaderData.append("\n");
		}
		Success = true;
	}
	return Success;
}