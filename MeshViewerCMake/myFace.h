#pragma once

class myHalfedge;
class myVector3D;

class myFace
{
public:
	myHalfedge *adjacent_halfedge;

	myVector3D *normal;

	void computeNormal();
	myFace(void);
	~myFace(void);
};