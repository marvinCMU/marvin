#include "Renderer.h"

Renderer::Renderer(void)
{
}

void Renderer::SetStaticStrucuture(vector<StaticStructure> pvStaticStructure)
{
	vStaticStructure = pvStaticStructure;
	StaticStructureCentering();
}

void Renderer::SetCamera(vector<Camera> pvCamera)
{
	vCamera = pvCamera;
	CameraCentering();
}

void Renderer::RenderStaticStructure()
{
	glBegin(GL_POINTS);
	for (int iStaticStructure = 0; iStaticStructure < vStaticStructure.size(); iStaticStructure++)
	{
		glColor3f((float)vStaticStructure[iStaticStructure].r/255,	(float)vStaticStructure[iStaticStructure].g/255, (float)vStaticStructure[iStaticStructure].b/255);
		glVertex3f(vStaticStructure[iStaticStructure].x,vStaticStructure[iStaticStructure].y, vStaticStructure[iStaticStructure].z);

	}
	glEnd();
}

void Renderer::RenderStaticCamera()
{
	glBegin(GL_LINES);
	for (int iCamera = 0; iCamera < vCamera.size(); iCamera++)
	{
		for (int iFrame = 0; iFrame < vCamera[iCamera].vC.size(); iFrame++)
		{
			float x = (float) cvGetReal2D(vCamera[iCamera].vC[iFrame], 0, 0);
			float y = (float) cvGetReal2D(vCamera[iCamera].vC[iFrame], 1, 0);
			float z = (float) cvGetReal2D(vCamera[iCamera].vC[iFrame], 2, 0);
			CvMat *R = cvCloneMat(vCamera[iCamera].vR[iFrame]);
			CvMat *C = cvCloneMat(vCamera[iCamera].vC[iFrame]);
			float r11 = (float) cvGetReal2D(R, 0, 0);	float r12 = (float) cvGetReal2D(R, 0, 1);	float r13 = (float) cvGetReal2D(R, 0, 2);
			float r21 = (float) cvGetReal2D(R, 1, 0);	float r22 = (float) cvGetReal2D(R, 1, 1);	float r23 = (float) cvGetReal2D(R, 1, 2);
			float r31 = (float) cvGetReal2D(R, 2, 0);	float r32 = (float) cvGetReal2D(R, 2, 1);	float r33 = (float) cvGetReal2D(R, 2, 2);
			float scale = 0.1;
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(x,y,z);	glVertex3f(x+scale*r11, y+scale*r12, z+scale*r13);	
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3f(x,y,z);	glVertex3f(x+scale*r21, y+scale*r22, z+scale*r23);	
			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3f(x,y,z);	glVertex3f(x+scale*r31, y+scale*r32, z+scale*r33);	
			CvMat *W11 = cvCreateMat(3,1,CV_32FC1);
			cvSetReal2D(W11, 0, 0, scale);	cvSetReal2D(W11, 1, 0, scale);	cvSetReal2D(W11, 2, 0, scale);
			CvMat *W12 = cvCreateMat(3,1,CV_32FC1);
			cvSetReal2D(W12, 0, 0, -scale);	cvSetReal2D(W12, 1, 0, scale);	cvSetReal2D(W12, 2, 0, scale);
			CvMat *W21 = cvCreateMat(3,1,CV_32FC1);
			cvSetReal2D(W21, 0, 0, scale);	cvSetReal2D(W21, 1, 0, -scale);	cvSetReal2D(W21, 2, 0, scale);
			CvMat *W22 = cvCreateMat(3,1,CV_32FC1);
			cvSetReal2D(W22, 0, 0, -scale);	cvSetReal2D(W22, 1, 0, -scale);	cvSetReal2D(W22, 2, 0, scale);
			cvInvert(R, R);
			cvMatMulAdd(R, W11, C, W11);
			cvMatMulAdd(R, W12, C, W12);
			cvMatMulAdd(R, W21, C, W21);
			cvMatMulAdd(R, W22, C, W22);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(cvGetReal2D(W11, 0, 0), cvGetReal2D(W11, 1, 0), cvGetReal2D(W11, 2, 0));
			glVertex3f(cvGetReal2D(W12, 0, 0), cvGetReal2D(W12, 1, 0), cvGetReal2D(W12, 2, 0));
			glVertex3f(cvGetReal2D(W12, 0, 0), cvGetReal2D(W12, 1, 0), cvGetReal2D(W12, 2, 0));
			glVertex3f(cvGetReal2D(W22, 0, 0), cvGetReal2D(W22, 1, 0), cvGetReal2D(W22, 2, 0));
			glVertex3f(cvGetReal2D(W22, 0, 0), cvGetReal2D(W22, 1, 0), cvGetReal2D(W22, 2, 0));
			glVertex3f(cvGetReal2D(W21, 0, 0), cvGetReal2D(W21, 1, 0), cvGetReal2D(W21, 2, 0));
			glVertex3f(cvGetReal2D(W21, 0, 0), cvGetReal2D(W21, 1, 0), cvGetReal2D(W21, 2, 0));
			glVertex3f(cvGetReal2D(W11, 0, 0), cvGetReal2D(W11, 1, 0), cvGetReal2D(W11, 2, 0));

			glVertex3f(x,y,z);
			glVertex3f(cvGetReal2D(W11, 0, 0), cvGetReal2D(W11, 1, 0), cvGetReal2D(W11, 2, 0));
			glVertex3f(x,y,z);
			glVertex3f(cvGetReal2D(W12, 0, 0), cvGetReal2D(W12, 1, 0), cvGetReal2D(W12, 2, 0));
			glVertex3f(x,y,z);
			glVertex3f(cvGetReal2D(W21, 0, 0), cvGetReal2D(W21, 1, 0), cvGetReal2D(W21, 2, 0));
			glVertex3f(x,y,z);
			glVertex3f(cvGetReal2D(W22, 0, 0), cvGetReal2D(W22, 1, 0), cvGetReal2D(W22, 2, 0));

		}
	}
	glEnd();
}

void Renderer::StaticStructureCentering()
{
	double average_x=0, average_y=0, average_z=0;
	for (int iStaticStructure = 0; iStaticStructure < vStaticStructure.size(); iStaticStructure++)
	{
		average_x += vStaticStructure[iStaticStructure].x;
		average_y += vStaticStructure[iStaticStructure].y;
		average_z += vStaticStructure[iStaticStructure].z;
	}
	average_x /= (double)vStaticStructure.size();
	average_y /= (double)vStaticStructure.size();
	average_z /= (double)vStaticStructure.size();
	for (int iStaticStrucutre = 0; iStaticStrucutre < vStaticStructure.size(); iStaticStrucutre++)
	{
		vStaticStructure[iStaticStrucutre].x -= average_x;
		vStaticStructure[iStaticStrucutre].y -= average_y;
		vStaticStructure[iStaticStrucutre].z -= average_z;
	}
	center_x = average_x;
	center_y = average_y;
	center_z = average_z;
}

void Renderer::CameraCentering()
{
	for (int iCamera = 0; iCamera < vCamera.size(); iCamera++)
	{
		for (int iFrame = 0; iFrame < vCamera[iCamera].vC.size(); iFrame++)
		{
			cvSetReal2D(vCamera[iCamera].vC[iFrame], 0, 0, cvGetReal2D(vCamera[iCamera].vC[iFrame], 0, 0)-center_x);
			cvSetReal2D(vCamera[iCamera].vC[iFrame], 1, 0, cvGetReal2D(vCamera[iCamera].vC[iFrame], 1, 0)-center_y);
			cvSetReal2D(vCamera[iCamera].vC[iFrame], 2, 0, cvGetReal2D(vCamera[iCamera].vC[iFrame], 2, 0)-center_z);
		}

	}
}

Renderer::~Renderer(void)
{
}
