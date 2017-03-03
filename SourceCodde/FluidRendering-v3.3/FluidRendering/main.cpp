#include <gl/glew.h>
#include <gl/freeglut.h>
#include <fstream>
#include <string>
#include "datastruct.h"
#include "ParticlesData.h"
#include "Shader.h"
#include "FluidRender.h"

SCamera gCamera;
SProjectInfo gProjectInfo;
SFluidParticle gFluidParticle;
CFluidRender gFluid;

GLfloat gLastX = 640.0f;
GLfloat gLastY = 360.0f;
GLfloat gLastB = -1;
GLfloat gYaw   = -90.0f;	// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat gPitch =   0.0f;

bool gFirstMouse = true;

void renderScene()
{
	glViewport(0,0,WIN_WIDTH, WIN_HEIGHT);
	gFluid.setCamera(gCamera);
	gFluid.setProjectInfo(gProjectInfo);
	gFluid.updateFluidParticle(gFluidParticle, 10000);
	gFluid.render();
	glutSwapBuffers();
}

void initializeScene()
{
	gFluidParticle.init(20000, 10);
	gFluid.initializeFluidRender();
	gCamera = SCamera(glm::vec3(1.0, 10.5, 10.0), glm::vec3(5.0, 5.0, -10.0), glm::vec3(0.0, 1.0, 0.0));
	gProjectInfo = SProjectInfo(90, WIN_WIDTH, WIN_HEIGHT, 0.02, 2000);
	
// 	std::string FileNames[6] = {
// 		"../Content/organic_rt.tga",
// 		"../Content/organic_lf.tga",
// 		"../Content/organic_up.tga",
// 		"../Content/organic_dn.tga",
// 		"../Content/organic_bk.tga",
// 		"../Content/organic_ft.tga"
// 	};
	std::string FileNames[6] = {
		"../Content/vendetta-cove_rt.tga",
		"../Content/vendetta-cove_lf.tga",
		"../Content/vendetta-cove_up.tga",
		"../Content/vendetta-cove_dn.tga",
		"../Content/vendetta-cove_bk.tga",
		"../Content/vendetta-cove_ft.tga"
	};

	gFluid.initializeCubemap(FileNames);

}

void keyboard(unsigned char	key, int x, int y)
{
	switch (key)
	{
	case 'w':
		gCamera.m_Eye.z -= 0.5;
		break;
	case 's':
		gCamera.m_Eye.z += 0.5;
		break;
	case 'q':
		gCamera.m_Eye.y += 0.5;
		break;
	case 'e':
		gCamera.m_Eye.y -= 0.5;
		break;
	case 'a':
		gCamera.m_Eye.x -= 0.5;
		break;
	case 'd':
		gCamera.m_Eye.x += 0.5;
		break;
	default:
		break;
	}
// 	GLfloat CameraSpeed = 0.05f;
// 
// 	switch (key)
// 	{
// 	case 'w':
// 		gCamera.m_Eye += CameraSpeed * gCamera.m_Target;
// 		break;
// 	case 's':
// 		gCamera.m_Eye -= CameraSpeed * gCamera.m_Target;
// 		break;
// 	case 'a':
// 		gCamera.m_Eye -= CameraSpeed * glm::normalize(glm::cross(gCamera.m_Target, gCamera.m_Up));
// 		break;
// 	case 'd':
// 		gCamera.m_Eye += CameraSpeed * glm::normalize(glm::cross(gCamera.m_Target, gCamera.m_Up));
// 		break;
// 	default:
// 		break;
// 	}
 	glutPostRedisplay();
}

//*********************************************************************************
//FUNCTION:
void onMouse(int button, int state, int x, int y)
{
	switch (state)
	{
	case GLUT_UP:
		{
			gLastX = x;
			gLastY = y;
			gLastB = false;
			break;
		}
	case GLUT_DOWN:
		{
			gLastX = x;
			gLastY = y;
			gLastB = button;
			break;
		}
	}
}

//*********************************************************************************
//FUNCTION:
void onMouseMotion(int x, int y)
{
	if(gFirstMouse)
	{
		gLastX = x;
		gLastY = y;
		gFirstMouse = false;
	}

	GLfloat XOffset = x - gLastX;
	GLfloat YOffset = gLastY - y; // 注意这里是相反的，因为y坐标的范围是从下往上的
	gLastX = x;
	gLastY = y;

	if (gLastB == GLUT_RIGHT_BUTTON)
	{
		GLfloat sensitivity = 0.05f;
		XOffset *= sensitivity;
		YOffset *= sensitivity;

		gYaw   += XOffset;
		gPitch += YOffset; 

		if (gPitch > 89.0f)
		{
			gPitch =  89.0f;
		}
		if (gPitch < -89.0f)
		{
			gPitch = -89.0f;
		}

		glm::vec3 CameraFront;
		CameraFront.x = cos(glm::radians(gPitch)) * cos(glm::radians(gYaw));
		CameraFront.y = sin(glm::radians(gPitch));
		CameraFront.z = cos(glm::radians(gPitch)) * sin(glm::radians(gYaw));
		gCamera.m_Target = glm::normalize(CameraFront);

		glutPostRedisplay();
	}
}

void initializeGlutCallback()
{
	glutDisplayFunc(renderScene);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(&onMouse);
	glutMotionFunc(&onMouseMotion);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Fluid Rendering ");

	GLuint res = glewInit();
	if (res != GLEW_OK)
	{
		fprintf(stderr, "Error '%s' \n", glewGetErrorString(res));
		return 1;
	}
	initializeGlutCallback();
	initializeScene();

	glutMainLoop();
	return 0;
}