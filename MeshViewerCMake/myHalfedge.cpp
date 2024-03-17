#include "myHalfedge.h"

myHalfedge::myHalfedge(void)
{
	source = nullptr;
	adjacent_face = nullptr;
	next = nullptr;
	prev = nullptr;
	twin = nullptr;
}


myHalfedge::~myHalfedge(void)
{
}
