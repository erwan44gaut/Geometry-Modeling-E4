#pragma once

class myVertex;
class myFace;
class myPoint3D;

class myHalfedge
{
public:
	myVertex *source; 
	myFace *adjacent_face; 
	myHalfedge *next;  
	myHalfedge *prev;  
	myHalfedge *twin;  

	myHalfedge(void);
	~myHalfedge(void);
};