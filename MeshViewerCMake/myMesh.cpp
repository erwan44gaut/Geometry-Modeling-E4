#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myVector3D.h"

using namespace std;

myMesh::myMesh(void)
{
	vertices.clear();
	faces.clear();
	halfedges.clear();
}


myMesh::~myMesh(void)
{
}

void myMesh::checkMesh()
{
}

void myMesh::readFile(std::string filename)
{
	string s, t;
	string tmp;

	ifstream fin(filename);
	if (!fin.is_open()) cout << "Unable to open file!\n";

	while ( getline(fin, s) )
	{
		stringstream myline(s);
		myline >> t;
		if (t == "v")
		{
			myline >> tmp;
			std::cout << "(" << stof(tmp.substr(0, tmp.find("/")));

			myline >> tmp;
			std::cout << ", " <<  stof(tmp.substr(0, tmp.find("/")));

			myline >> tmp;
			std::cout << ", " <<  stof(tmp.substr(0, tmp.find("/"))) << ")\n";
		}
		if (t == "f")
		{
			while (myline >> tmp) std::cout << stoi(tmp.substr(0, tmp.find("/"))) -1 << " -> ";
			std::cout << std::endl;
		}
	}
	checkMesh();
	//normalize(); 
}

void myMesh::computeNormals()
{

}

void myMesh::normalize()
{
	int i;
	int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

	for (i=0;i<vertices.size();i++) {
		if (vertices[i]->point->X < vertices[tmpxmin]->point->X) tmpxmin = i;
		if (vertices[i]->point->X > vertices[tmpxmax]->point->X) tmpxmax = i;

		if (vertices[i]->point->Y < vertices[tmpymin]->point->Y) tmpymin = i;
		if (vertices[i]->point->Y > vertices[tmpymax]->point->Y) tmpymax = i;

		if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z) tmpzmin = i;
		if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z) tmpzmax = i;
	}

	double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X, 
		ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y, 
		zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

	double scale =  (xmax-xmin) > (ymax-ymin) ? (xmax-xmin) : (ymax-ymin);
	scale = scale > (zmax-zmin) ? scale : (zmax-zmin);

	for (i=0;i<vertices.size();i++) {
		vertices[i]->point->X -= (xmax+xmin)/2;
		vertices[i]->point->Y -= (ymax+ymin)/2;
		vertices[i]->point->Z -= (zmax+zmin)/2;

		vertices[i]->point->X /= scale;
		vertices[i]->point->Y /= scale;
		vertices[i]->point->Z /= scale;
	}
}
