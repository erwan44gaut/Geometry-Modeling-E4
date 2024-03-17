#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myvector3d.h"

using namespace std;

myMesh::myMesh(void)
{
	/**** TODO ****/
}

myMesh::~myMesh(void)
{
	for (auto vertex : vertices) {
		delete vertex;
	}
	for (auto halfedge : halfedges) {
		delete halfedge;
	}
	for (auto face : faces) {
		delete face;
	}
}

void myMesh::clear()
{
	for (unsigned int i = 0; i < vertices.size(); i++) if (vertices[i]) delete vertices[i];
	for (unsigned int i = 0; i < halfedges.size(); i++) if (halfedges[i]) delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++) if (faces[i]) delete faces[i];

	vector<myVertex *> empty_vertices;    vertices.swap(empty_vertices);
	vector<myHalfedge *> empty_halfedges; halfedges.swap(empty_halfedges);
	vector<myFace *> empty_faces;         faces.swap(empty_faces);
}

void myMesh::checkMesh()
{
	int countNullNext = 0;
	int countNullPrev = 0;
	int countNullTwin = 0;
	int countNullFace = 0;
	int countNullSource = 0;
	int countIncoherentNext = 0;
	int countIncoherentPrev = 0;
	int countIncoherentTwin = 0;

	// Vérification des demi-arêtes
	for (myHalfedge* he : halfedges)
	{
		if (he->twin == nullptr) countNullTwin++;
		if (he->next == nullptr) countNullNext++;
		if (he->prev == nullptr) countNullPrev++;
		if (he->source == nullptr) countNullSource++;
		if (he->adjacent_face == nullptr) countNullFace++;
		if (he->next->prev != he) countIncoherentNext++;
		if (he->prev->next != he) countIncoherentPrev++;
		if (he->twin && he->twin->twin != he) countIncoherentTwin++;
	}

	int countNullOriginOf = 0;
	int countIncoherentOriginOf = 0;

	// Vérification des sommets
	for (myVertex* vertex : vertices)
	{
		if (vertex->originof == nullptr) countNullOriginOf++;
		if (vertex->originof != nullptr && vertex->originof->source != vertex) countIncoherentOriginOf++;
	}

	int countIncoherentFace = 0;

	// Vérification des faces
	for (myFace* face : faces)
	{
		myHalfedge* startHalfedge = face->adjacent_halfedge;
		myHalfedge* currentHalfedge = startHalfedge;
		bool isFaceLoopValid = true;

		do
		{
			if (currentHalfedge->next->prev != currentHalfedge || currentHalfedge->prev->next != currentHalfedge)
			{
				isFaceLoopValid = false;
				break;
			}
			currentHalfedge = currentHalfedge->next;
		} while (currentHalfedge != startHalfedge);

		if (!isFaceLoopValid) countIncoherentFace++;
	}

	// Affichage des erreurs
	if (countNullNext > 0)
		std::cout << "Error! " << countNullNext << " half-edges have no 'next' reference.\n";
	if (countNullPrev > 0)
		std::cout << "Error! " << countNullPrev << " half-edges have no 'prev' reference.\n";
	if (countNullTwin > 0)
		std::cout << "Error! " << countNullTwin << " half-edges have no 'twin' reference.\n";
	if (countNullFace > 0)
		std::cout << "Error! " << countNullFace << " half-edges have no 'face' reference.\n";
	if (countNullSource > 0)
		std::cout << "Error! " << countNullSource << " half-edges have no 'source' reference.\n";
	if (countIncoherentNext > 0)
		std::cout << "Error! " << countIncoherentNext << " half-edges have incoherent 'next' references.\n";
	if (countIncoherentPrev > 0)
		std::cout << "Error! " << countIncoherentPrev << " half-edges have incoherent 'prev' references.\n";
	if (countIncoherentTwin > 0)
		std::cout << "Error! " << countIncoherentTwin << " half-edges have incoherent 'twin' references.\n";
	if (countNullOriginOf > 0)
		std::cout << "Error! " << countNullOriginOf << " vertices have no 'originof' reference.\n";
	if (countIncoherentOriginOf > 0)
		std::cout << "Error! " << countIncoherentOriginOf << " vertices have incoherent 'originof' references.\n";
	if (countIncoherentFace > 0)
		std::cout << "Error! " << countIncoherentFace << " faces have incoherent half-edge loops.\n";

	if (countNullNext + countNullPrev + countNullTwin + countNullFace + countNullSource + countIncoherentNext + countIncoherentPrev + countIncoherentTwin + countNullOriginOf + countIncoherentOriginOf + countIncoherentFace == 0)
		std::cout << "Mesh integrity check passed: All half-edges, vertices, and faces are correctly linked.\n";
}

bool myMesh::readFile(std::string filename)
{
	string s, t, u;
	vector<int> faceids;
	myHalfedge ** hedges;

	ifstream fin(filename);
	if (!fin.is_open()) {
		cout << "Unable to open file!\n";
		return false;
	}
	name = filename;

	map<pair<int, int>, myHalfedge *> twin_map;
	map<pair<int, int>, myHalfedge *>::iterator it;

	while (getline(fin, s))
	{
		stringstream myline(s);
		myline >> t;
		if (t == "g") {}
		else if (t == "v")
		{
			float x, y, z;
			myline >> x >> y >> z;
			cout << "v " << x << " " << y << " " << z << endl;

			myPoint3D *point = new myPoint3D(x, y, z);
			myVertex *vertex = new myVertex();
			vertex->point = point;
			vertices.push_back(vertex);
		}
		else if (t == "mtllib") {}
		else if (t == "usemtl") {}
		else if (t == "s") {}
		else if (t == "f")
		{
			faceids.clear();
			// Parsing
			while (myline >> u)
				faceids.push_back(atoi((u.substr(0, u.find("/"))).c_str()) - 1);

			if (faceids.size() < 3) continue;

			int faceidsSize = faceids.size();
			hedges = new myHalfedge * [faceidsSize]; // allocate the array for storing pointers to half-edges
			
			for (size_t i = 0; i < faceidsSize; i++)
				hedges[i] = new myHalfedge();

			myFace* face = new myFace();
			face->adjacent_halfedge = hedges[0];

			for (size_t i = 0; i < faceidsSize; i++)
			{
				int iplusone = (i + 1) % faceidsSize;
				int iminusone = (i - 1 + faceidsSize) % faceidsSize;

				// connect prevs, and next
				hedges[i]->next = hedges[iplusone];
				hedges[i]->prev = hedges[iminusone];

				// search for the twins using twin_map
				it = twin_map.find(make_pair(faceids[iplusone], faceids[i]));

				if (it == twin_map.end())
				{
					// Si la twin existe
					twin_map[make_pair(faceids[i], faceids[iplusone])] = hedges[i];
				}
				else
				{
					// Si la twin n'existe pas
					auto twin = it->second;
					twin->twin = hedges[i];
					hedges[i]->twin = twin;
				}

				// set originof
				hedges[i]->source = vertices[faceids[i]];
				vertices[faceids[i]]->originof = hedges[i];

				hedges[i]->adjacent_face = face;

				// push edges to halfedges in myMesh
				halfedges.push_back(hedges[i]);
			}
			// push faces to faces in myMesh
			faces.push_back(face);
			delete[] hedges;
		}
	}

	checkMesh();
	normalize();

	return true;
}

void myMesh::computeNormals()
{
	for (auto& face : faces) {
		face->computeNormal();
	}

	for (auto& vertex : vertices) {
		vertex->computeNormal();
	}
}

void myMesh::normalize()
{
	if (vertices.size() < 1) return;

	int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

	for (unsigned int i = 0; i < vertices.size(); i++) {
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

	double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
	scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

	for (unsigned int i = 0; i < vertices.size(); i++) {
		vertices[i]->point->X -= (xmax + xmin) / 2;
		vertices[i]->point->Y -= (ymax + ymin) / 2;
		vertices[i]->point->Z -= (zmax + zmin) / 2;

		vertices[i]->point->X /= scale;
		vertices[i]->point->Y /= scale;
		vertices[i]->point->Z /= scale;
	}
}

void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p)
{

	/**** TODO ****/
}

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}

void myMesh::subdivisionCatmullClark()
{
	std::vector<myVertex*> newVertices; // Vecteur pour stocker les points de sommet
	std::vector<myHalfedge*> newHalfedges; // Vecteur pour stocker les demi-arêtes
	std::vector<myFace*> newFaces; // Vecteur pour stocker les faces
	std::map<myFace*, myVertex*>facePoints; // Vecteur pour stocker les barycentres (points de face)
	std::map<myHalfedge*, myVertex*>edgePoints; // Vecteur pour stocker les points d'arête
	std::map<myVertex*, myVertex*> vertexPoints; // Vecteur pour stocker les nouveaux sommets
	std::map<std::pair<myVertex*, myVertex*>, myHalfedge*> halfedgeMap;

	for (auto& face : faces)
	{
		// Initialisation pour parcourir les halfedges de la face.
		myHalfedge* startEdge = face->adjacent_halfedge;
		myHalfedge* currentEdge = startEdge;

		// Calcul du barycentre de la face.
		myPoint3D* barycenter = new myPoint3D(0.0, 0.0, 0.0);
		int count = 0;

		do {
			*barycenter += *currentEdge->source->point;
			currentEdge = currentEdge->next;
			count++;
		} while (currentEdge != startEdge);

		if (count > 0) {
			*barycenter /= count;
		}

		// Crée un nouveau sommet au barycentre.
		myVertex* barycenterVertex = new myVertex();
		barycenterVertex->point = barycenter;

		facePoints[face] = barycenterVertex; // Ajoute le point de face au vecteur
		newVertices.push_back(barycenterVertex); // Ajoute le point de face au vecteur
	}

	for (auto& halfedge : halfedges)
	{
		if (halfedge < halfedge->twin) continue; // Ne pas traiter les demi-arêtes jumelles deux fois

		myPoint3D* edgePoint = new myPoint3D(0, 0, 0);

		// Point moyen des extrémités de l'arête
		myPoint3D* V1 = halfedge->source->point;
		myPoint3D* V2 = halfedge->next->source->point;

		// Point moyen des points de face adjacents
		myPoint3D* P1 = facePoints[halfedge->adjacent_face]->point;
		myPoint3D* P2 = facePoints[halfedge->twin->adjacent_face]->point;

		// Moyenne de ces quatre points
		*edgePoint = (*V1 + *V2 + *P1 + *P2) / 4;

		myVertex* edgeVertex = new myVertex();
		edgeVertex->point = edgePoint;

		// Affecte le point d'arête aux demi-arêtes jumelles'
		edgePoints[halfedge] = edgeVertex;
		edgePoints[halfedge->twin] = edgeVertex;

		newVertices.push_back(edgeVertex); // Ajoute le point d'arête au vecteur
	}

	for (auto& vertice : vertices) {
		myPoint3D* F = new myPoint3D(0, 0, 0); // Moyenne des points de face
		myPoint3D* R = new myPoint3D(0, 0, 0); // Moyenne des milieux d'arête
		int n = 0; // Nombre de faces/arêtes touchant le sommet

		// Initialisation pour parcourir les demi-arêtes du sommet.
		myHalfedge* startEdge = vertice->originof;
		myHalfedge* currentEdge = startEdge;

		do {
			myFace* newFace = currentEdge->adjacent_face;
			myVertex* facePoint = facePoints[newFace];
			*F += *facePoint->point;

			myPoint3D* endPoint = currentEdge->twin->source->point;
			myPoint3D* startPoint = currentEdge->source->point;
			myPoint3D* moy = new myPoint3D((*endPoint + *startPoint) / 2);
			*R += *moy;

			currentEdge = currentEdge->twin->next;
			n++;
		} while (currentEdge != startEdge);

		if (n > 0) {
			*F /= n;
			*R /= n;
		}

		// Calculer la nouvelle position du sommet.
		myPoint3D* newPosition = new myPoint3D();
		*newPosition = (*F + *R * 2 + *vertice->point * (n - 3)) / n;

		myVertex* newVertex = new myVertex();
		newVertex->point = newPosition;

		vertexPoints[vertice] = newVertex;
		newVertices.push_back(newVertex); // Ajoute le nouveau sommet au vecteur
	}

	for (auto& vertice : vertices)
	{
		// Initialisation pour parcourir les demi-arêtes du sommet.
		myHalfedge* startEdge = vertice->originof;
		myHalfedge* currentEdge = startEdge;

		myVertex* newVertex = vertexPoints[vertice];

		do
		{
			myVertex* edgePoint = edgePoints[currentEdge];
			myHalfedge* newHalfedge = new myHalfedge();
			newHalfedge->source = edgePoint;
			myHalfedge* newTwin = new myHalfedge();
			newTwin->source = newVertex;
			newHalfedge->twin = newTwin;
			newTwin->twin = newHalfedge;

			newHalfedges.push_back(newHalfedge);
			newHalfedges.push_back(newTwin);

			halfedgeMap[std::make_pair(edgePoint, newVertex)] = newHalfedge;
			halfedgeMap[std::make_pair(newVertex, edgePoint)] = newTwin;

			currentEdge = currentEdge->twin->next;
		} while (currentEdge != startEdge);
	}

	for (auto& face : faces)
	{
		// Initialisation pour parcourir les demi-arêtes de la face.
		myHalfedge* startEdge = face->adjacent_halfedge;
		myHalfedge* currentEdge = startEdge;

		myVertex* newVertex = facePoints[face];

		do
		{
			myVertex* edgePoint = edgePoints[currentEdge];
			myHalfedge* newHalfedge = new myHalfedge();
			newHalfedge->source = edgePoint;
			myHalfedge* newTwin = new myHalfedge();
			newTwin->source = newVertex;
			newHalfedge->twin = newTwin;
			newTwin->twin = newHalfedge;

			newHalfedges.push_back(newHalfedge);
			newHalfedges.push_back(newTwin);

			halfedgeMap[std::make_pair(edgePoint, newVertex)] = newHalfedge;
			halfedgeMap[std::make_pair(newVertex, edgePoint)] = newTwin;

			currentEdge = currentEdge->next;
		} while (currentEdge != startEdge);
	}

	for (auto& face : faces)
	{
		// Initialisation pour parcourir les demi-arêtes de la face.
		myHalfedge* startEdge = face->adjacent_halfedge;
		myHalfedge* currentEdge = startEdge;

		myVertex* newVertex = facePoints[face];

		do
		{
			myVertex* edgePoint = edgePoints[currentEdge];
			myVertex* prevEdgeVertex = edgePoints[currentEdge->prev];
			myVertex* vertexPoint = vertexPoints[currentEdge->source];

			myFace* newFace = new myFace();

			myHalfedge* firstHalfedge = halfedgeMap[std::make_pair(vertexPoint, edgePoint)];
			myHalfedge* secondHalfedge = halfedgeMap[std::make_pair(edgePoint, newVertex)];
			myHalfedge* thirdHalfedge = halfedgeMap[std::make_pair(newVertex, prevEdgeVertex)];
			myHalfedge* fourthHalfedge = halfedgeMap[std::make_pair(prevEdgeVertex, vertexPoint)];

			firstHalfedge->next = secondHalfedge;
			secondHalfedge->next = thirdHalfedge;
			thirdHalfedge->next = fourthHalfedge;
			fourthHalfedge->next = firstHalfedge;

			firstHalfedge->prev = fourthHalfedge;
			secondHalfedge->prev = firstHalfedge;
			thirdHalfedge->prev = secondHalfedge;
			fourthHalfedge->prev = thirdHalfedge;

			firstHalfedge->adjacent_face = newFace;
			secondHalfedge->adjacent_face = newFace;
			thirdHalfedge->adjacent_face = newFace;
			fourthHalfedge->adjacent_face = newFace;

			newVertex->originof = thirdHalfedge;
			edgePoint->originof = secondHalfedge;
			vertexPoint->originof = firstHalfedge;
			prevEdgeVertex->originof = fourthHalfedge;

			newFace->adjacent_halfedge = firstHalfedge;

			newFaces.push_back(newFace);

			currentEdge = currentEdge->next;
		} while (currentEdge != startEdge);
	}

	clear();

	vertices = newVertices;
	halfedges = newHalfedges;
	faces = newFaces;
}

void myMesh::triangulate()
{
	std::vector<myFace*> newFaces;  // Crée un vecteur pour stocker les nouvelles faces triangulées.

	// Parcours de chaque face du maillage.
	for (auto& face : faces)
	{
		// Vérifie si la face peut être triangulée.
		if (triangulate(face))
		{
			// Initialisation pour parcourir les halfedges de la face.
			myHalfedge* startEdge = face->adjacent_halfedge;
			myHalfedge* currentEdge = startEdge;

			// Calcul du barycentre de la face.
			myPoint3D* barycenter = new myPoint3D(0.0, 0.0, 0.0);
			int count = 0;

			do {
				*barycenter += *currentEdge->source->point;
				currentEdge = currentEdge->next;
				count++;
			} while (currentEdge != startEdge);

			if (count > 0) {
				*barycenter /= count;
			}

			// Crée un nouveau sommet au barycentre.
			myVertex* barycenterVertex = new myVertex();
			barycenterVertex->point = barycenter;

			// Préparation pour la construction de nouvelles halfedges.
			myHalfedge* twinEndToCenter = new myHalfedge;
			myHalfedge* nextCurrent = new myHalfedge();

			currentEdge = startEdge;

			// Boucle pour créer de nouvelles faces triangulées.
			do
			{
				nextCurrent = currentEdge->next;

				// Création d'une nouvelle face triangulaire.
				myFace* newTriangle = new myFace();
				newTriangle->adjacent_halfedge = currentEdge;

				// Création de nouvelles halfedges pour le triangle.
				myHalfedge* endToCenter = new myHalfedge();
				myHalfedge* centerToStart = new myHalfedge();
				barycenterVertex->originof = centerToStart;

				newTriangle->adjacent_halfedge = endToCenter;

				// Configuration des halfedges du triangle.
				endToCenter->source = currentEdge->next->source;
				endToCenter->adjacent_face = newTriangle;
				endToCenter->prev = currentEdge;
				endToCenter->next = centerToStart;

				centerToStart->source = barycenterVertex;
				centerToStart->adjacent_face = newTriangle;
				centerToStart->prev = endToCenter;
				centerToStart->next = currentEdge;
				centerToStart->twin = twinEndToCenter;

				currentEdge->adjacent_face = newTriangle;
				currentEdge->prev = centerToStart;
				currentEdge->next = endToCenter;

				twinEndToCenter->twin = centerToStart;
				twinEndToCenter = endToCenter;

				// Avancer à la halfedge suivante.
				currentEdge = nextCurrent;

				// Ajout des halfedges et de la nouvelle face au maillage.
				halfedges.push_back(endToCenter);
				halfedges.push_back(centerToStart);
				newFaces.push_back(newTriangle);

			} while (currentEdge != startEdge);

			// Finalisation de la triangulation pour la face courante.
			startEdge->prev->twin = twinEndToCenter;
			twinEndToCenter->twin = startEdge->prev;
		}
		else
		{
			// Si la face ne peut pas être triangulée, elle est conservée telle quelle.
			newFaces.push_back(face);
		}
	}

	// Mise à jour du maillage avec les nouvelles faces triangulées.
	faces = newFaces;
}

/*
* return false if already triangle, true othewise.
*/
bool myMesh::triangulate(myFace *f)
{
	if (f->adjacent_halfedge == f->adjacent_halfedge->next->next->next) return false;
	return true;
}

void myMesh::simplification()
{
	// Calcule de la plus petite halfedge (arête la plus courte).	
	float smallestHalfedgeDistance = fabs(halfedges[0]->source->point - halfedges[0]->next->source->point);
	myHalfedge* smallestHalfedge = halfedges[0];

	// Parcourir toutes les halfedges pour trouver la plus courte.
	for (auto& halfedge : halfedges)
	{
		float halfedgeDistance = fabs(halfedge->source->point - halfedge->next->source->point);

		if (halfedgeDistance < smallestHalfedgeDistance)
		{
			smallestHalfedgeDistance = halfedgeDistance;
			smallestHalfedge = halfedge;
		}
	}

	// Fusion des sommets v1 et v2 de la halfedge la plus courte en un point moyen.
	myVertex* v1 = smallestHalfedge->source;
	myVertex* v2 = smallestHalfedge->next->source;
	*v1->point = (*v1->point + *v2->point) / 2.0;

	// Réaffectation des halfedges de v2 à v1.
	myHalfedge* startHalfedge = smallestHalfedge->twin;
	myHalfedge* currentHalfedge = startHalfedge;

	do
	{
		currentHalfedge->source = v1;
		currentHalfedge->source->originof = currentHalfedge;
		currentHalfedge = currentHalfedge->twin->next;
	} while (startHalfedge != currentHalfedge);

	bool flag_1 = false;
	bool flag_2 = false;

	// Simplification de la face adjacente à la halfedge la plus courte.
	if (!triangulate(smallestHalfedge->adjacent_face))
	{
		smallestHalfedge->prev->twin->twin = smallestHalfedge->next->twin;
		smallestHalfedge->next->twin->twin = smallestHalfedge->prev->twin;
		smallestHalfedge->prev->source->originof = smallestHalfedge->next->twin;
		flag_1 = true;	
	}
	else
	{
		smallestHalfedge->prev->next = smallestHalfedge->next;
		smallestHalfedge->next->prev = smallestHalfedge->prev;
		smallestHalfedge->adjacent_face->adjacent_halfedge = smallestHalfedge->next;
	}

	// Simplification de la face adjacente à la halfedge jumelle.
	if (!triangulate(smallestHalfedge->twin->adjacent_face))
	{
		smallestHalfedge->twin->prev->twin->twin = smallestHalfedge->twin->next->twin;
		smallestHalfedge->twin->next->twin->twin = smallestHalfedge->twin->prev->twin;
		smallestHalfedge->twin->prev->source->originof = smallestHalfedge->twin->next->twin;
		flag_2 = true;
	}
	else
	{
		smallestHalfedge->twin->prev->next = smallestHalfedge->twin->next;
		smallestHalfedge->twin->next->prev = smallestHalfedge->twin->prev;
		smallestHalfedge->twin->adjacent_face->adjacent_halfedge = smallestHalfedge->twin->next;
	}

	// Suppression des faces simplifiées.
	faces.erase(std::remove_if(faces.begin(), faces.end(),
		[&smallestHalfedge, flag_1, flag_2](const auto& face)
		{
			return (face == smallestHalfedge->adjacent_face && flag_1) ||
				(face == smallestHalfedge->twin->adjacent_face && flag_2);
		}),
		faces.end());

	// Suppression des halfedges et des vertices qui ne sont plus nécessaires.
	if (flag_1)
	{
		halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), smallestHalfedge->next), halfedges.end());
		halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), smallestHalfedge->prev), halfedges.end());
	}
	
	if (flag_2)
	{
		halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), smallestHalfedge->twin->next), halfedges.end());
		halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), smallestHalfedge->twin->prev), halfedges.end());
	}

	halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), smallestHalfedge->twin), halfedges.end());
	halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), smallestHalfedge), halfedges.end());
	vertices.erase(std::remove(vertices.begin(), vertices.end(), v2), vertices.end());

	checkMesh();
}