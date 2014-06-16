#ifndef RENDERUTILITY_H
#define RENDERUTILITY_H
#include "structdefinition.h"
#include <GL/freeglut.h>

void RenderStaticStructure(vector<StaticStructure> vStaticStructure);
void StaticStructureCentering(vector<StaticStructure> &vStaticStructure, double &center_x, double &center_y, double &center_z);

#endif //RENDERUTILITY_H
