#include <math.h>

#ifndef __INCLUDEPOINT3D
#define __INCLUDEPOINT3D

class myVector3D;

class myPoint3D
{
  //The x,y,z coordinates of this point.
public:

  double X, Y, Z;

  myPoint3D();

  myPoint3D(double x, double y, double z);
  double dist(myPoint3D p1);
  myPoint3D operator+( const myVector3D & v1);
  myPoint3D & operator+=( const myVector3D & v1);
  myVector3D operator-( const myPoint3D & p1);
  void rotate( const myVector3D & lp, double theta);
  void print(char *s);
  double dist(myPoint3D *p1, myPoint3D *p2);
  double dist(myPoint3D *p1, myPoint3D *p2, myPoint3D *p3);
};

#endif


