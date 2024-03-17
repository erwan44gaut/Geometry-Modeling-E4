#include "myVertex.h"
#include "myvector3d.h"
#include "myHalfedge.h"
#include "myFace.h"

myVertex::myVertex(void)
{
	point = NULL;
	originof = NULL;
	normal = new myVector3D(1.0,1.0,1.0);
}

myVertex::~myVertex(void)
{
	if (normal) delete normal;
}

void myVertex::computeNormal()
{
    myHalfedge* halfedge = originof; // Commence par la halfedge d'origine du sommet.
    myHalfedge* current = halfedge; // Utilise 'current' pour parcourir les halfedges.
    myVector3D normalSum(0.0, 0.0, 0.0); // Initialise un vecteur pour la somme des normales.
    int count = 0; // Compteur pour le nombre de faces adjacentes.

    do {
        myFace* face = current->adjacent_face; // Obtient la face adjacente � la halfedge courante.

        normalSum += *face->normal; // Ajoute la normale de la face � la somme totale.

        current = current->twin->next; // Passe � la halfedge jumelle de la suivante pour continuer autour du sommet.
        count++; // Incr�mente le compteur pour chaque face adjacente.
    } while (halfedge != current); // Continue jusqu'� ce qu'elle revienne � la halfedge d'origine.

    if (count > 0) {
        normalSum = normalSum / (double)count; // Moyenne les normales si le compteur est sup�rieur � 0.
    }
    else {
        normalSum = myVector3D(0.0, 0.0, 0.0); // Dans le cas contraire, d�finit la normale � z�ro.
    }

    *normal = normalSum; // Affecte la normale moyenn�e au sommet.
}

