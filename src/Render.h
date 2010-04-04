#include "Geometry.h"
#include "Image.h"
#include "Mesh.h"




#ifndef RENDER_H
#define RENDER_H



enum CullMode
{
	CULL_FRONT,
	CULL_BACK,
	CULL_NONE
};

void render(
	const Mesh&,
	const Matrix4&,
	const list<SunLight>&,
	Image&,
	CullMode = CULL_NONE);



#endif