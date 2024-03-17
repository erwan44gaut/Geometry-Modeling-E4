#include "myPoint3D.h"
#include "myVector3D.h"
#include <iostream>

myPoint3D::myPoint3D() {}

myPoint3D::myPoint3D(double x, double y, double z)
{
    X = x;
    Y = y;
    Z = z; 
}

myPoint3D myPoint3D::operator+( const myVector3D & v1)
{
	return myPoint3D(X+v1.dX, Y+v1.dY, Z+v1.dZ);
}

myPoint3D & myPoint3D::operator+=( const myVector3D & v1)
{
	X += v1.dX;
	Y += v1.dY;
	Z += v1.dZ;
	return *this;
}

myVector3D myPoint3D::operator-( const myPoint3D & p1)
{
	return myVector3D( X-p1.X, Y-p1.Y, Z-p1.Z );
}

double myPoint3D::dist(myPoint3D p1)
{
	return 0.0;
}

void myPoint3D::rotate( const myVector3D & lp, double theta)
{
	myVector3D tmp(X, Y, Z);
	tmp.rotate(lp, theta);
	X = tmp.dX; Y = tmp.dY; Z = tmp.dZ;
}

void myPoint3D::print(char *s)
{
	  std::cout << s << X << ", " << Y << ", " << Z << "\n";
}

double myPoint3D::dist(myPoint3D *p1, myPoint3D *p2)
{
	return 0.0;
}

double myPoint3D::dist(myPoint3D *p1, myPoint3D *p2, myPoint3D *p3)
{
	return 0.0;
}