#include "RenderUtility.h"

void DisplayStaticStructure(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderStaticStructure(vStaticStructure);
	glutSwapBuffers();
}

void RenderStaticStructure(vector<StaticStructure> vStaticStructure)
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_POINTS);
	for (int iStaticStructure = 0; iStaticStructure < vStaticStructure.size(); iStaticStructure++)
	{
		glColor3f((float)vStaticStructure[iStaticStructure].r/255,	(float)vStaticStructure[iStaticStructure].g/255, (float)vStaticStructure[iStaticStructure].b/255);
		glVertex3f(vStaticStructure[iStaticStructure].x,vStaticStructure[iStaticStructure].y, vStaticStructure[iStaticStructure].z);
		
	}
	glEnd();
	//glutSwapBuffers();
}

void StaticStructureCentering(vector<StaticStructure> &vStaticStructure, double &center_x, double &center_y, double &center_z)
{
	double average_x=0, average_y=0, average_z=0;
	for (int iStaticStructure = 0; iStaticStructure < vx.size(); iStaticStructure++)
	{
		StaticStructure ss;
		ss.id = visibleID[iStaticStructure];
		ss.x = vx[iStaticStructure];
		ss.y = vy[iStaticStructure];
		ss.z = vz[iStaticStructure];
		average_x += ss.x;
		average_y += ss.y;
		average_z += ss.z;
		ss.r = vr[iStaticStructure];
		ss.g = vg[iStaticStructure];
		ss.b = vb[iStaticStructure];
		vStaticStructure.push_back(ss);
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
}