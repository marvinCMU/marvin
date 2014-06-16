#ifndef MOUSENAVIGATION_H
#define MOUSENAVIGATION_H

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/freeglut.h>
#include "structdefinition.h"

#ifdef __cplusplus
extern "C"
{
#endif
	void MouseNavigation_Init();

	extern GLfloat ReferencePoint[4];

	void MouseNavigation_SelectionFunc(void (*f)(vector<StaticStructure>), vector<StaticStructure>);      /* Selection-mode draw function */
	extern void MouseNavigation_PickFunc(void (*f)(GLint name));     /* Pick event handling function */

#ifdef __cplusplus
}
#endif

#endif