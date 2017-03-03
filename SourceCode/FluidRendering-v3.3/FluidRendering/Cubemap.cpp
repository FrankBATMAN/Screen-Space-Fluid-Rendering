#include "Cubemap.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

CCubemap::CCubemap(void)
{
	__initCubemap();
}


CCubemap::~CCubemap(void)
{
	delete m_pShader;
}

//******************************************************************
//FUNCTION:
bool CCubemap::LoadCubeMap(const std::string* vFileNames)
{
	glGenTextures(1, &m_CubemapTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTex);

	for (int i=0; i<6; i++)
	{
		GLuint  ImageType;
		GLubyte	*ImageData = NULL;
		GLuint  ImageWidth;
		GLuint  ImageHeight;
		if (ImageData != 0x0 ) free (ImageData);
		if (!__loadTGA(vFileNames[i], ImageData, ImageWidth, ImageHeight, ImageType))
		{
			return false;
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, ImageType, ImageWidth, ImageHeight, 0, ImageType, GL_UNSIGNED_BYTE, ImageData);

		free (ImageData);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return true;
}

//******************************************************************
//FUNCTION:
void CCubemap::initializeCubemap(const SCamera& vCamera, const SProjectInfo& vProjection)
{
	m_pShader->enable();
	glm::mat4 WorldTransformMat = glm::mat4(1.0);
//	glm::mat4 ViewTransformMat = glm::lookAt(vCamera.m_Eye, vCamera.m_Target, vCamera.m_Up);
	glm::mat4 ViewTransformMat = glm::lookAt(vCamera.m_Eye, vCamera.m_Eye + vCamera.m_Target, vCamera.m_Up);
	glm::mat4 View =  glm::mat4(glm::mat3(ViewTransformMat));
	float Aspect = (float)vProjection.m_WindosWidth / (float)vProjection.m_WindosHeight;
	glm::mat4 ProjectionTransformMat = glm::perspective(vProjection.m_Fov, Aspect, vProjection.m_Near, vProjection.m_Far);
	glm::mat4 WVPTransformMat = ProjectionTransformMat * View * WorldTransformMat;

	GLuint WVPTransformLocation = m_pShader->getUniformLocation("uWVPTransformMat");
	glUniformMatrix4fv(WVPTransformLocation, 1, GL_FALSE, glm::value_ptr(WVPTransformMat));

	GLuint SkyBoxSamplerLocation = m_pShader->getUniformLocation("uSkyBoxSampler");
	glUniform1i(SkyBoxSamplerLocation, 0);
	m_pShader->ban();
}

//******************************************************************
//FUNCTION:
void CCubemap::draw()
{
	m_pShader->enable();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTex);
	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);
	m_pShader->ban();
}

//******************************************************************
//FUNCTION:
void CCubemap::bindTexture(GLenum vTexUnit)
{
	glActiveTexture(vTexUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTex);
}

//******************************************************************
//FUNCTION:
bool CCubemap::__loadTGA(const std::string& vFileName, GLubyte* &voImageData, GLuint& voWidth, GLuint& voHeight, GLenum& voType)
{
	GLubyte     TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};    // Uncompressed TGA Header  
	GLubyte     TGAcompare[12];                             // Used To Compare TGA Header  
	GLubyte     header[6];                                  // First 6 Useful Bytes From The Header  
	GLuint      bytesPerPixel;                              // Holds Number Of Bytes Per Pixel Used In The TGA File  
	GLuint      imageSize;                                  // Used To Store The Image Size When Setting Aside Ram  
	GLuint      temp;                                       // Temporary Variable  
	GLuint      type=GL_RGBA;                               // Set The Default GL Mode To RBGA (32 BPP)
	//	GLubyte		*ImageData;

	FILE *file = fopen(vFileName.c_str(), "rb");                     // Open The TGA File  

	if( file==NULL ||                                       // Does File Even Exist?  
		fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||  // Are There 12 Bytes To Read?  
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0               ||  // Does The Header Match What We Want?  
		fread(header,1,sizeof(header),file)!=sizeof(header))                // If So Read Next 6 Header Bytes  
	{  
		if (file == NULL)                                   // Did The File Even Exist? *Added Jim Strong*  
			return false;                                   // Return False  
		else  
		{  
			fclose(file);                                   // If Anything Failed, Close The File  
			return false;                                   // Return False  
		}  
	} 

	GLuint Width  = header[1] * 256 + header[0];
	GLuint Height = header[3] * 256 + header[2];

	//只能使用24位或者32位的TGA图像 
	if (Width<=0 || Height<=0 || (header[4]!=24 && header[4]!=32))
	{
		fclose(file);
		return false;
	}

	GLubyte BitPerPixel = header[4];
	bytesPerPixel = BitPerPixel / 8;
	imageSize = Width * Height * bytesPerPixel;

	if (voImageData != 0x0 ) free (voImageData);
	voImageData = (GLubyte *)malloc(imageSize);

	if( voImageData==NULL ||                          // Does The Storage Memory Exist?  
		fread(voImageData, 1, imageSize, file)!=imageSize)    // Does The Image Size Match The Memory Reserved?  
	{  
		if(voImageData!=NULL)                     // Was Image Data Loaded  
			free(voImageData);                        // If So, Release The Image Data  

		fclose(file);                                       // Close The File  
		return false;                                       // Return False  
	}  

	//RGB数据格式转换，便于在OpenGL中使用 ,TGA图像中数据存放的顺序是BGR(A)，而在OpenGL中顺序是RGB(A)，所以在进行纹理生成的时候必须先进行格式的转化。
	for(GLuint i=0; i<int(imageSize); i+=bytesPerPixel)      // Loop Through The Image Data  
	{                                                       // Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)  
		temp=voImageData[i];                          // Temporarily Store The Value At Image Data 'i'  
		voImageData[i] = voImageData[i + 2];    // Set The 1st Byte To The Value Of The 3rd Byte  
		voImageData[i + 2] = temp;                    // Set The 3rd Byte To The Value In 'temp' (1st Byte Value)  
	}  

	fclose (file);

	if (BitPerPixel==24)                                 // Was The TGA 24 Bits  
	{  
		type=GL_RGB;                                        // If So Set The 'type' To GL_RGB  
	}

	voWidth = Width;
	voHeight = Height;
	voType = type;
	return true;
}

//******************************************************************
//FUNCTION:
void CCubemap::__initCubemap()
{
	m_pShader = new CShader;
	m_pShader->addShader(GL_VERTEX_SHADER, "cubemapV.glsl");
	m_pShader->addShader(GL_FRAGMENT_SHADER, "cubemapF.glsl");
	m_pShader->compileShader();

	glm::vec3 SkyBoxVertices[8] =
	{
		glm::vec3(-1.0f, -1.0f, 1.0f),
		glm::vec3( 1.0f, -1.0f, 1.0f),
		glm::vec3( 1.0f, 1.0f, 1.0f),
		glm::vec3(-1.0f, 1.0f, 1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, 1.0f, -1.0f),
		glm::vec3( 1.0f, 1.0f, -1.0f),
		glm::vec3( 1.0f, -1.0f, -1.0f)
	};

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SkyBoxVertices), SkyBoxVertices, GL_STATIC_DRAW);

	unsigned int Indices[] =
	{
		1, 2, 6, 7,
		0, 3, 5, 4,
		2, 3, 5, 6,
		0, 1, 7, 4,
		0, 1, 2, 3,
		7, 6, 5, 4 
	};

	glGenBuffers(1, &m_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}