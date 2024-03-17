#include "myVertex.h"
#include "myVector3D.h"
#include "myHalfedge.h"
#include "myFace.h"

myVertex::myVertex(void)
{
	point = new myPoint3D();
	originof = nullptr;
	normal = new myVector3D();
}

myVertex::~myVertex(void)
{
}

void myVertex::computeNormal()
{
}