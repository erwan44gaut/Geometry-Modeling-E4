#include <fstream>
#include <GL/glew.h>
#include <GL/freeglut.h>

using namespace std;

#include "shaders.h"

#include "myVector3D.h"
#include "myPoint3D.h"
#include "myMesh.h"

#define MAX 10000000.0

GLuint vertexshader, fragmentshader, shaderprogram ; // shaders

// width and height of the window.
int Glut_w = 600, Glut_h = 400; 

//Variables and their values for the camera setup.
myPoint3D camera_eye(0,0,1);
myVector3D camera_up(0,1,0);
myVector3D camera_forward (0,0,-1);

float fovy = 90;
float zNear = 0.1;
float zFar = 60;

int button_pressed = 0; // 1 if a button is currently being pressed.
int GLUTmouse[2] = { 0, 0 };
enum MENU { MENU_CATMULLCLARK, MENU_WIREFRAME, MENU_EXIT, MENU_MESH, MENU_LOOP, MENU_DRAWMESHVERTICES,
		    MENU_CONTRACTEDGE, MENU_CONTRACTFACE, MENU_CREASE, MENU_SILHOUETTE, 
			MENU_GENERATE, MENU_CUT, MENU_INFLATE, MENU_SELECTEDGE, MENU_SELECTFACE, MENU_SELECTVERTEX,
			MENU_SHADING, MENU_SMOOTHEN, MENU_SPLITEDGE, MENU_SPLITFACE, MENU_SELECTCLEAR, 
			MENU_TRIANGULATE, MENU_UNDO, MENU_WRITE, MENU_SIMPLIFY, MENU_NORMALS};

myMesh *m;
myPoint3D *pickedpoint;
myHalfedge *closest_edge;
myVertex *closest_vertex;
myFace *closest_face;

bool smooth = false; //smooth = true means smooth normals, default false means face-wise normals.
bool drawmesh = true;
bool drawwireframe = false;
bool drawmeshvertices = false;
bool drawsilhouette = false;
bool drawnormals = false;

void draw_text(GLfloat x, GLfloat y, GLfloat z, string text)
{
	GLfloat w = 1;
	GLfloat fx, fy;

	glColor3f(0.9, 0.0, 0.0);

	glPushAttrib( GL_TRANSFORM_BIT | GL_VIEWPORT_BIT );

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	glDepthRange( z, z );
	glViewport( (int) x - 1, (int) y - 1, 2, 2 );

	fx = x - (int) x;
	fy = y - (int) y;
	glRasterPos4f( fx, fy, 0.0, w );

	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	glPopAttrib();
	
	glUseProgram(0);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char *)text.c_str());
	glUseProgram(shaderprogram);
}

bool PickedPoint(int x, int y) 
{
	// Check position
	if ((x < 0) || (Glut_w <= x) || (y < 0) || (Glut_h <= y)) { 
		printf("Pick (%d,%d) outside viewport: (0,%d) (0,%d)\n", x, y, Glut_w, Glut_h); 
		return false;
	}

	// Allocate select buffer
	const int SELECT_BUFFER_SIZE = 1024;
	GLuint select_buffer[SELECT_BUFFER_SIZE];
	GLint select_buffer_hits;

	// Initialize select buffer
	glSelectBuffer(SELECT_BUFFER_SIZE, select_buffer);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	// Initialize view transformation
	GLint viewport[4];
	glViewport(0, 0, Glut_w, Glut_h);
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix((GLdouble) x, (GLdouble) y, 1, 1, viewport);
	// Set projection transformation
	// NOTE: THIS MUST MATCH CODE IN GLUTRedraw
	gluPerspective(fovy, Glut_w/(float)Glut_h, zNear, zFar);

	// Set camera transformation
	// NOTE: THIS MUST MATCH CODE IN GLUTRedraw
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera_eye.X, camera_eye.Y, camera_eye.Z, camera_eye.X + camera_forward.dX, camera_eye.Y + camera_forward.dY, camera_eye.Z + camera_forward.dZ, camera_up.dX, camera_up.dY, camera_up.dZ);

	// Draw mesh with pick names into selection buffer
	glLoadName(0); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int i = 0;
	for (vector<myFace *>::iterator it = m->faces.begin(); it != m->faces.end(); it++)
	{
		glLoadName(++i); 
		glBegin(GL_POLYGON);
		myHalfedge *e = (*it)->adjacent_halfedge;
		myVertex *v;

		v = e->source;
		glVertex3f( v->point->X, v->point->Y, v->point->Z);

		e = e->next; v = e->source;
		glVertex3f( v->point->X, v->point->Y, v->point->Z);

		e = e->next; v = e->source;
		glVertex3f( v->point->X, v->point->Y, v->point->Z);
		glEnd();

	}

	glFlush();
	select_buffer_hits = glRenderMode(GL_RENDER);

	// Process select buffer to find front-most hit
	int hit = 0;
	GLuint hit_z = 0xFFFFFFFF;
	GLuint *bufp = select_buffer;
	GLuint numnames, z1, z2;
	for (int i = 0; i < select_buffer_hits; i++) {
		numnames = *bufp++;
		z1 = *bufp++;
		z2 = *bufp++;
		while (numnames--) {
			if (z1 < hit_z) {
				hit = (int) *bufp;
				hit_z = z1/2 + z2/2;
			}
			bufp++;
		}
	}

	// Return closest face
	if ((hit > 0) ) {
		// Find face
		//if (pick_face) {
		// Subtract one because added one in glLoadName
		// *pick_face = &mesh->t[hit-1];

		// }
		closest_face = m->faces[hit-1];
		// Find hit position
		if (pickedpoint) {
			GLdouble p[3];
			GLdouble modelview_matrix[16];
			GLdouble projection_matrix[16];
			GLint viewport[16];
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
			glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
			glGetIntegerv(GL_VIEWPORT, viewport);
			GLdouble z = (GLdouble) hit_z / (GLdouble) 0xFFFFFFFF;
			gluUnProject(x, y, z, modelview_matrix, projection_matrix, viewport, &(p[0]), &(p[1]), &(p[2]));
			pickedpoint->X = p[0];
			pickedpoint->Y = p[1];
			pickedpoint->Z = p[2];
			//pick_position->set(p[0], p[1], p[2]);
		}

		// Return hit
		cout << "true\n";
		return true;
	}
	else {
		// Return no hit
		cout << "false\n";
		return false;
	}
}

void menu(int item)
{
	switch(item)
	{
		case MENU_SHADING:
		{
			smooth = !smooth;
			break;
		}
		case MENU_MESH:
		{
			drawmesh = !drawmesh;
			break;
		}
		case MENU_DRAWMESHVERTICES:
		{
			drawmeshvertices = !drawmeshvertices;
			break;
		}
		case MENU_WIREFRAME:
		{
			drawwireframe = !drawwireframe;
			break;
		}
		case MENU_NORMALS:
		{
			drawnormals = !drawnormals;
			break;
		}
		case MENU_SILHOUETTE:
		{
			drawsilhouette = !drawsilhouette;
			break;
		}
		case MENU_SELECTCLEAR:
		{
			closest_edge = NULL;
			closest_vertex = NULL;
			closest_face = NULL;
			break;
		}
		case MENU_SELECTEDGE:
		{
			if (pickedpoint == NULL) break;
			//write code to select closest edge to pickedpoint
			break;
		}
		case MENU_SELECTVERTEX:
		{
			if (pickedpoint == NULL) break;
			//write code to select closest vertex to pickedpoint
			break;
		}
		case MENU_SELECTFACE:
		{
			if (pickedpoint == NULL) break;
			//write code to select closest face to pickedpoint
			break;
		}
		case MENU_EXIT:
		{
			exit(0);
			break;
		}
	}
	glutPostRedisplay();
}

//This function is called when a mouse button is pressed.
void mouse(int button, int state, int x, int y)
{
  // Remember button state 
  button_pressed = (state == GLUT_DOWN) ? 1 : 0;

   // Remember mouse position 
  GLUTmouse[0] = x;
  GLUTmouse[1] = Glut_h - y;

    // Process mouse button event
  if (state == GLUT_DOWN) 
  {
    if (button == GLUT_LEFT_BUTTON) {
		int mode = glutGetModifiers();
		if (mode == GLUT_ACTIVE_CTRL) 
		{
			glUseProgram(0);
			pickedpoint = new myPoint3D();
			if (!PickedPoint(x, Glut_h-y)) delete pickedpoint;
			glUseProgram(shaderprogram);
		}
	}
    else if (button == GLUT_MIDDLE_BUTTON) {}
    else if (button == GLUT_RIGHT_BUTTON) {}
  }

  glutPostRedisplay();
}

//This function is called when the mouse is dragged.
void mousedrag(int x, int y)
{
  // Invert y coordinate
  y = Glut_h - y;

  //change in the mouse position since last time
  int dx = x - GLUTmouse[0];
  int dy = y - GLUTmouse[1];

  GLUTmouse[0] = x;
  GLUTmouse[1] = y;

  if (dx == 0 && dy == 0) return;
  if (button_pressed == 0) return;

  double vx = (double) dx / (double) Glut_w;
  double vy = (double) dy / (double) Glut_h;
  double theta = 4.0 * (fabs(vx) + fabs(vy));

  myVector3D camera_right = camera_forward.crossproduct(camera_up);
  camera_right.normalize();

  myVector3D tomovein_direction = -camera_right * vx + -camera_up * vy;

  myVector3D rotation_axis = tomovein_direction.crossproduct(camera_forward);
  rotation_axis.normalize();
  
  camera_forward.rotate(rotation_axis, theta);
  camera_up.rotate(rotation_axis, theta);
  camera_eye.rotate(rotation_axis, theta);
 
  camera_up.normalize();
  camera_forward.normalize();

  glutPostRedisplay();
}

void mouseWheel(int button, int dir, int x, int y)
{
    if (dir > 0)
        camera_eye += camera_forward * 0.1;
    else
        camera_eye += -camera_forward * 0.1;
	glutPostRedisplay();
}

//This function is called when a key is pressed.
void keyboard(unsigned char key, int x, int y) {
	switch(key) {
	case 27:  // Escape to quit
		exit(0) ;
        break ;
	case 'h':
		cout << "The keys for various algorithms are:\n";
		break;
	case '1':
		smooth = !smooth;
		break;
	}
	glutPostRedisplay();
}

//This function is called when an arrow key is pressed.
void keyboard2(int key, int x, int y) {
	switch(key) {
	case GLUT_KEY_UP:
		camera_eye += camera_forward * 0.1;
		break;
	case GLUT_KEY_DOWN:
		camera_eye += -camera_forward * 0.1;
		break;
	case GLUT_KEY_LEFT:
		camera_up.normalize();
		camera_forward.rotate(camera_up, 0.1);
		camera_forward.normalize();
		break;
	case GLUT_KEY_RIGHT:
		camera_up.normalize();
		camera_forward.rotate(camera_up, -0.1);
		camera_forward.normalize();
		break;
	}
	glutPostRedisplay();
}


void reshape(int width, int height){
	Glut_w = width;
	Glut_h = height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, Glut_w/(float)Glut_h, zNear, zFar);
	glViewport(0, 0, Glut_w, Glut_h);
	glutPostRedisplay();
}

//This function is called to display objects on screen.
void display() 
{
	//Clearing the color on the screen.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Setting up the projection matrix.
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, Glut_w/(float)Glut_h, zNear, zFar);
	glViewport(0, 0, Glut_w, Glut_h);

	//Setting up the modelview matrix.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera_eye.X, camera_eye.Y, camera_eye.Z, camera_eye.X + camera_forward.dX, camera_eye.Y + camera_forward.dY, camera_eye.Z + camera_forward.dZ, camera_up.dX, camera_up.dY, camera_up.dZ);

	if (drawmesh || drawsilhouette){
		glLineWidth(1.0);
		glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset(2.0f, 2.0f); //for z-bleeding, z-fighting.
		if (drawsilhouette && !drawmesh) glUseProgram(0);
		glBegin(GL_TRIANGLES);
		if (drawmesh) glColor3f(0.4f,0.8f,0.4f);
		else glColor3f(1.0,1.0,1.0);
		for (vector<myFace *>::iterator it = m->faces.begin(); it != m->faces.end(); it++)
		{
			myHalfedge *estart = (*it)->adjacent_halfedge;
			myHalfedge *e = estart->next;
			myVertex *vstart = estart->source;
			while (e->next != estart)
			{
				myVertex *v;
				myVector3D r;
				v = vstart;
				if (smooth) r = *(v->normal);
				else r = *(*it)->normal;
				glNormal3f(r.dX, r.dY, r.dZ);
				glVertex3f( v->point->X, v->point->Y, v->point->Z);

				v = e->source;
				if (smooth) r = *(v->normal);
				else r = *(*it)->normal;
				glNormal3f(r.dX, r.dY, r.dZ);
				glVertex3f( v->point->X, v->point->Y, v->point->Z);

				v = e->next->source;
				if (smooth) r = *(v->normal);
				else r = *(*it)->normal;
				glNormal3f(r.dX, r.dY, r.dZ);
				glVertex3f( v->point->X, v->point->Y, v->point->Z);

				e = e->next;
			}
		}
		glEnd();
		if (drawsilhouette) glUseProgram(shaderprogram);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	if (drawmeshvertices)
	{
		glUseProgram(0);
		glPointSize(2.0);
		glBegin(GL_POINTS);
		glColor3f(0,0,0);
		for (unsigned int i=0;i<m->vertices.size();i++)
			glVertex3f(m->vertices[i]->point->X, m->vertices[i]->point->Y, m->vertices[i]->point->Z);
		glEnd();
		glUseProgram(shaderprogram);
	}

	if (drawwireframe)
	{
		glLineWidth(2.0);
		glBegin(GL_LINES);
		glColor3f(0.0,0.0,0.0);
		for (vector<myHalfedge *>::iterator it = m->halfedges.begin(); it != m->halfedges.end(); it++)
		{
			myHalfedge *e = (*it);
			myVertex *v1 = (*it)->source;
			myVertex *v2 = (*it)->next->source;

			glNormal3f(v1->normal->dX, v1->normal->dY, v1->normal->dZ);
			glVertex3f( v1->point->X , v1->point->Y, v1->point->Z);
			glNormal3f(v2->normal->dX, v2->normal->dY, v2->normal->dZ);
			glVertex3f( v2->point->X, v2->point->Y, v2->point->Z );
		}
		glEnd();
	}

	if (drawsilhouette)
	{
	}


	if (drawnormals)
	{
		glUseProgram(0);
		glLineWidth(1.0);
		glBegin(GL_LINES);
		glColor3f(0.2f,0.2f,0.2f);
		for (vector<myVertex *>::iterator it = m->vertices.begin(); it != m->vertices.end(); it++)
		{
			myVertex *v = *it;
			myVector3D *normal = v->normal;

			glVertex3f( v->point->X, v->point->Y, v->point->Z);
			glVertex3f( v->point->X + 0.02*normal->dX, v->point->Y + 0.02*normal->dY, v->point->Z+ 0.02*normal->dZ);
		}
		glEnd();
		glUseProgram(shaderprogram);
	}


	if (pickedpoint != NULL)
	{
		glUseProgram(0);
		glPointSize(8.0);
		glBegin(GL_POINTS);
		glColor3f(1,0,1);
		glVertex3f(pickedpoint->X, pickedpoint->Y, pickedpoint->Z);
		glEnd();
		glUseProgram(shaderprogram);
	}

	if (closest_edge != NULL)
	{
		glUseProgram(0);
		glLineWidth(4.0);
		glBegin(GL_LINES);
		glColor3f(1,0,1);
		glVertex3f(closest_edge->source->point->X, closest_edge->source->point->Y, closest_edge->source->point->Z);
		glVertex3f(closest_edge->twin->source->point->X, closest_edge->twin->source->point->Y, closest_edge->twin->source->point->Z);
		glEnd();
		glUseProgram(shaderprogram);
	}
	
	if (closest_vertex != NULL)
	{
		glUseProgram(0);
		glPointSize(8.0);
		glBegin(GL_POINTS);
		glColor3f(0,0,0);
		glVertex3f(closest_vertex->point->X, closest_vertex->point->Y, closest_vertex->point->Z);
		glEnd();
		glUseProgram(shaderprogram);
	}
	
	if (closest_face != NULL)
	{
		glUseProgram(0);
		glBegin(GL_TRIANGLES);
		glColor3f(0.1,0.1,0.9);
		myHalfedge *e = closest_face->adjacent_halfedge;
		myVertex *v;
		myVector3D r;

		v = e->source;
		glVertex3f( v->point->X, v->point->Y, v->point->Z);
		e = e->next; v = e->source;
		glVertex3f( v->point->X, v->point->Y, v->point->Z);
		e = e->next; v = e->source;
		glVertex3f( v->point->X, v->point->Y, v->point->Z);

		glEnd();
		glUseProgram(shaderprogram);
	}

	draw_text(0.0f,1.0f, 0, "Vertices:    " + to_string(static_cast<long long>(m->vertices.size())) );
	draw_text(0.0f,22.0f, 0,"Halfedges: " + to_string(static_cast<long long>(m->halfedges.size())) );
	draw_text(0.0f,41.0f, 0,"Faces:       " + to_string(static_cast<long long>(m->faces.size())) );

	glFlush();
}

//This function is called from the main to initalize everything.
void init()
{
    vertexshader = initshaders(GL_VERTEX_SHADER, "./shaders/light.vert.glsl") ;
    fragmentshader = initshaders(GL_FRAGMENT_SHADER, "./shaders/light.frag.glsl") ;
    shaderprogram = initprogram(vertexshader, fragmentshader) ; 

	pickedpoint = NULL;
	closest_edge = NULL;
	closest_vertex = NULL;
	closest_face = NULL;

	m = new myMesh();

	//Fixing the halfedge data structure to contain a triangle.
	//Should be replaced with: m->readFile("apple.obj") to read a Mesh from an obj file.
	{
		for (int i=0;i<3;i++) 
			m->vertices.push_back(new myVertex());
		for (int i=0;i<3;i++) 
			m->halfedges.push_back(new myHalfedge());
		m->faces.push_back(new myFace());

		m->faces[0]->adjacent_halfedge = m->halfedges[0];

		m->vertices[0]->point = new myPoint3D(0, 0, 0);
		m->vertices[1]->point = new myPoint3D(1, 0, 0);
		m->vertices[2]->point = new myPoint3D(0, 1, 0);

		for (int i=0;i<3;i++)
			m->vertices[i]->originof = m->halfedges[i];

		for (int i=0;i<3;i++) {
			m->halfedges[i]->adjacent_face = m->faces[0];
			m->halfedges[i]->next = m->halfedges[(i+1)%3];
			m->halfedges[i]->prev = m->halfedges[(i-1+3)%3];
			m->halfedges[i]->source = m->vertices[i];
			m->halfedges[i]->twin = NULL;
		}
	}
	m->readFile("apple.obj");
	//m->computeNormals();
}


int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutCreateWindow("My OpenGL Application");
	   
    glewInit();
	glutReshapeWindow(Glut_w, Glut_h);
	
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard2);
	glutMotionFunc(mousedrag) ;
	glutMouseFunc(mouse) ;
	glutMouseWheelFunc(mouseWheel);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc (GL_LESS) ;

	glClearColor(1,1,1,0);

	glEnable(GL_MULTISAMPLE);

	init();

	int sm1 = glutCreateMenu(menu);
	glutAddMenuEntry("Vertex-shading/Face-shading", MENU_SHADING);  
	glutAddMenuEntry("Mesh", MENU_MESH);  
	glutAddMenuEntry("Wireframe", MENU_WIREFRAME);  
	glutAddMenuEntry("Vertices", MENU_DRAWMESHVERTICES);  
	glutAddMenuEntry("Normals", MENU_NORMALS);  
	glutAddMenuEntry("Silhouette", MENU_SILHOUETTE);  
	glutAddMenuEntry("Crease", MENU_CREASE);

	int sm2 = glutCreateMenu(menu);
	glutAddMenuEntry("Catmull-Clark subdivision", MENU_CATMULLCLARK);
	glutAddMenuEntry("Loop subdivision", MENU_LOOP);
	glutAddMenuEntry("Inflate", MENU_INFLATE);
	glutAddMenuEntry("Smoothen", MENU_SMOOTHEN);
	glutAddMenuEntry("Simplification", MENU_SIMPLIFY);

	int sm3 = glutCreateMenu(menu);
	glutAddMenuEntry("Closest Edge", MENU_SELECTEDGE);
	glutAddMenuEntry("Closest Face", MENU_SELECTFACE);
	glutAddMenuEntry("Closest Vertex", MENU_SELECTVERTEX);
	glutAddMenuEntry("Clear", MENU_SELECTCLEAR);

	int sm4 = glutCreateMenu(menu);
	glutAddMenuEntry("Split edge", MENU_SPLITEDGE);
	glutAddMenuEntry("Split face", MENU_SPLITFACE);
	glutAddMenuEntry("Contract edge", MENU_CONTRACTEDGE);
	glutAddMenuEntry("Contract face", MENU_CONTRACTFACE);

	int m = glutCreateMenu(menu);
	glutAddSubMenu("Draw", sm1);
	glutAddSubMenu("Mesh Operations", sm2);
	glutAddSubMenu("Select", sm3);
	glutAddSubMenu("Face Operations", sm4);
	glutAddMenuEntry("Triangulate", MENU_TRIANGULATE);
	glutAddMenuEntry("Write to File", MENU_WRITE);
	glutAddMenuEntry("Undo", MENU_UNDO);
	glutAddMenuEntry("Generate Mesh", MENU_GENERATE);
	glutAddMenuEntry("Cut Mesh", MENU_CUT);
    glutAddMenuEntry("Exit", MENU_EXIT);     

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutMainLoop();
	return 0;
}
