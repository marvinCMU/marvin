#ifndef RENDERER_H
#define RENDERER_H
#include <vector>
#include <gl/freeglut.h>
#include "structdefinition.h"

class Renderer
{
public:
	Renderer(void);
	void SetStaticStrucuture(vector<StaticStructure> vStaticStructure);
	void SetCamera(vector<Camera> pvCamera);
	void RenderStaticStructure();
	void RenderStaticCamera();
	void StaticStructureCentering();
	void CameraCentering();
	
public:
	~Renderer(void);

	vector<StaticStructure> vStaticStructure;
	vector<Camera> vCamera;
	double center_x, center_y, center_z;
};

#endif //RENDERER_H