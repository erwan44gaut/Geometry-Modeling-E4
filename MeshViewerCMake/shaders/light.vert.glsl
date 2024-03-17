

varying vec3 mynormal;
varying vec4 myvertex;
varying vec4 mycolor;

void main() {
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex ; 
	mynormal = gl_Normal;
	myvertex = gl_Vertex;
	mycolor = gl_Color;
}
