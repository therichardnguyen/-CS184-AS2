#include "main.h"
#include <sstream>

using namespace std;

//****************************************************
// Some Classes
//****************************************************
class Viewport {
public:
    Viewport(): mousePos(0.0,0.0) { /* empty */ };
	int w, h; // width and height
	vec2 mousePos;
};

//****************************************************
// Global Variables
//****************************************************
Viewport viewport;
Polygon * polygon;
Vertex * tempVertex, * pickedV;
vector<Vertex*> pickedGroup;
bool picked = false, showMorph = false,output100 = false, 
	groupSelect = false, groupDrag = false, showBoundingBox = false,
	forward = true, circleVertices = false, displayCoordinates = false;
GLdouble vpx = 1.0, vpy = 1.0;	
double t = 1.0, sliderPos = 0.0; // interpolation factor
double gsxMin, gsxMax, gsyMin, gsyMax; // group Select box bounds
const double pickingLimitDefault = .03;
double pickingLimit = pickingLimitDefault;	// bounds of our picking box
unsigned int polysOutputed = 0;
vec2 vporigin(0.0,0.0);
UCB::ImageSaver * imgSaver;


// MY FUNCTIONS 

double distance(double x1, double y1, double x2, double y2) {
	return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

void zoomin() {
	if (vpx >= 0.2 || vpy >= 0.2) {
		vporigin[0] += viewport.mousePos[0]*.1;
		vporigin[1] += viewport.mousePos[1]*.1;
		vpx -= 0.1;
		vpy -= 0.1;
		pickingLimit -= pickingLimitDefault*(.1);
	}
	//Set up the PROJECTION transformationstack to be a simple orthographic [-1, +1] projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glOrtho(vporigin[0]-vpx, vporigin[0]+vpx, vporigin[1]-vpy, vporigin[1]+vpy, 1, -1);	// zoom in 

	//Set the MODELVIEW transformation stack to the identity.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glutPostRedisplay(); 
}

void zoomout() {
	if (vpx <1.0 || vpy <1.0) {
		vporigin[0] -= viewport.mousePos[0]*.1;
		vporigin[1] -= viewport.mousePos[1]*.1;
		vpx += 0.1;
		vpy += 0.1;
		pickingLimit += pickingLimitDefault*(.1);
	}
	//Set up the PROJECTION transformationstack to be a simple orthographic [-1, +1] projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glOrtho(vporigin[0]-vpx, vporigin[0]+vpx, vporigin[1]-vpy, vporigin[1]+vpy, 1, -1);	// zoom in 

	//Set the MODELVIEW transformation stack to the identity.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glutPostRedisplay(); 
			
}

void drawWindowBorder() {
	glColor3f(0,0,0);
	glBegin(GL_POLYGON);
	glVertex2d(-1.0,1.0);
	glVertex2d(1.0,1.0);
	glVertex2d(1.0,-1.0);
	glVertex2d(-1.0,-1.0);
	glEnd();
}

void typeCoords(double x, double y) {
	unsigned int i;
	stringstream convert;
	convert.precision(4);
	string coord="(";
	convert << fixed << x;
	coord.append(convert.str());
	coord.append(",");
	convert.str("");
	convert << fixed << y;
	coord.append(convert.str());
	coord.append(")");
	glPushMatrix();
	glColor3f(1.0,0,0);
	glRasterPos2d(x,y-.05*vpy);
	for (i=0;i<coord.size();i++)
	      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, coord.c_str()[i]);
	glPopMatrix();
	
}

void drawGSbox() { // draw the box for group selection

	glColor4f(0.2,0.2,0.2,0.7);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_POLYGON);
	glVertex2d(gsxMin,gsyMax);
	glVertex2d(gsxMax,gsyMax);
	glVertex2d(gsxMax,gsyMin);
	glVertex2d(gsxMin,gsyMin);
	glEnd();
	glDisable(GL_BLEND);
	
	glLineStipple(1,(short)0x5555);
	glEnable(GL_LINE_STIPPLE);
	glColor3f(0.8,0.8,0.8);
	glBegin(GL_LINE_LOOP);
	glVertex2d(gsxMin,gsyMax);
	glVertex2d(gsxMax,gsyMax);
	glVertex2d(gsxMax,gsyMin);
	glVertex2d(gsxMin,gsyMin);
	glDisable(GL_LINE_STIPPLE);
	glEnd();
	
}

bool canMoveVertical() {
	unsigned int i;
	vec2 mousePos(viewport.mousePos[0]*vpx+vporigin[0],viewport.mousePos[1]*vpy+vporigin[1]);
	for (i=0;i<pickedGroup.size();i++) {
		Vertex * v = pickedGroup.at(i);
		vec2 test = mousePos - tempVertex->getPos() + v->getPos(1.0);
		if (abs(test[1]) > 1.0 )
			return false;
	}
	return true;
}

bool canMoveHorizontal() {
	unsigned int i;
	vec2 mousePos(viewport.mousePos[0]*vpx+vporigin[0],viewport.mousePos[1]*vpy+vporigin[1]);
	for (i=0;i<pickedGroup.size();i++) {
		Vertex * v = pickedGroup.at(i);
		vec2 test = mousePos - tempVertex->getPos() + v->getPos(1.0);
		if (abs(test[0]) > 1.0 )
			return false;
	}
	return true;
}

//-------------------------------------------------------------------------------
/// You will be calling all of your drawing-related code from this function.
/// Nowhere else in your code should you use glBegin(...) and glEnd() except code
/// called from this method.
///
/// To force a redraw of the screen (eg. after mouse events or the like) simply call
/// glutPostRedisplay();

void display() {
	//Clear Buffers
    glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);					// indicate we are specifying camera transformations
	glLoadIdentity();							// make sure transformation is "zero'd"

	drawWindowBorder();
	if (!showMorph)
    	polygon->draw();
	polygon->draw(t);
	if (displayCoordinates) {
		unsigned int i;
		for (i=0;i<pickedGroup.size();i++) {
		 	typeCoords(pickedGroup.at(i)->getPos(1.0)[0],pickedGroup.at(i)->getPos(1.0)[1]);
		}
	}
	if (showBoundingBox){
		polygon->drawBoundingBox(t);
	}
	if (circleVertices &&!output100){
		polygon->circleVertices(pickingLimit,pickedGroup,t);
	}
	if (groupSelect) // for group select when implemented
		drawGSbox();
	//Now that we've drawn on the buffer, swap the drawing buffer and the displaying buffer.
	glutSwapBuffers();
	if (output100) {
		if (forward) {
			if (t < 1.0) {
				imgSaver->saveFrame(viewport.w, viewport.h);
				t+=.01;
				glutPostRedisplay();
			} else {
				output100=showMorph=showBoundingBox=false;
				glutPostRedisplay();
			}
		} else {
			if (t > 0.0) {
				imgSaver->saveFrame(viewport.w, viewport.h);
				t-=.01;
				glutPostRedisplay();
			} else {
				output100=showMorph=showBoundingBox=false;
				glutPostRedisplay();
			}
		}
	}
}

//-------------------------------------------------------------------------------
/// \brief	Called when the screen gets resized.
/// This gives you the opportunity to set up all the relevant transforms.
///

void reshape(int w, int h) {
	//Set up the viewport to ignore any size change.
	viewport.w = min(w,h);
	viewport.h = min(w,h);
	if (min(w,h) == w) 
		glViewport(0,(h-w)/2,viewport.w,viewport.h);
	else 
		glViewport((w-h)/2,0,viewport.w,viewport.h);

	//Set up the PROJECTION transformationstack to be a simple orthographic [-1, +1] projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glOrtho(vporigin[0]-vpx, vporigin[0]+vpx, vporigin[1]-vpy, vporigin[1]+vpy, 1, -1);	// zoom in 

	//Set the MODELVIEW transformation stack to the identity.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


//-------------------------------------------------------------------------------
/// Called to handle keyboard events.
void myKeyboardFunc (unsigned char key, int x, int y) {
	string f = "polygon";
	stringstream convert;
	if (!output100) {
		switch (key) {
			case 27:			// Escape key
				exit(0);
				break;
			case 115:			// 's' key
				convert << polysOutputed;
				if (polysOutputed != 0) {
					f.append(convert.str());
				}
				f.append(".obj");
				polygon->writeAsOBJ(f, 1.0); 
				break;
			case 97:				// 'a' key
				forward = output100 = showMorph = showBoundingBox = true;
				t=0.0;
				glutPostRedisplay();
				break;
			case 65:					// 'shift+a' key
				output100 = showMorph = showBoundingBox = true;
				forward = false;
				t=1.0;
				glutPostRedisplay();
				break;
			case 119:			// 'w' key
				zoomout();
				break;
			case 116:			// 't' key
				zoomin();
				break;
			case 114:			// 'r' key
				polygon->morphCopy();
				glutPostRedisplay();
				break;
			case 99:				// 'c' key
				vporigin[0] = vporigin[1] = 0;
				vpx = vpy = 1;
				pickingLimit = pickingLimitDefault;
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
			    glOrtho(-1,1,-1, 1, 1, -1);	// zoom in 

				//Set the MODELVIEW transformation stack to the identity.
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glutPostRedisplay();
				break;	
			case 118:				// 'v' key
				circleVertices = !circleVertices;
				glutPostRedisplay();
				break;
			case 111:				// 'o' key
				displayCoordinates = !displayCoordinates;
				glutPostRedisplay();
				break;
		}
	}
}


//-------------------------------------------------------------------------------
/// This function is used to convert the pixel coordinates on the screen to the
/// coordinates of your view volume. Thus, it converts a pixel location to a coordinate
/// that can be used for picking and selection.


vec2 inverseViewportTransform(vec2 screenCoords) {
    //Create a vec2 on the local stack. See algebra3.h
    vec2 viewCoords(0.0,0.0);
	GLdouble params[4];
	glGetDoublev(GL_VIEWPORT, params);
    viewCoords[0] = ((float)screenCoords[0] - (float) params[0] - viewport.w/2)/((float)(viewport.w/2));
    viewCoords[1] = ((float)screenCoords[1] - (float) params[1] - viewport.h/2)/((float)(viewport.h/2));
    //Flip the values to get the correct position relative to our coordinate axis.
    viewCoords[1] = -viewCoords[1];

    //C++ will copy the whole vec2 to the calling function.
    return viewCoords;
}

vec2 viewportTransform(vec2 viewCoords) {
	vec2 screenCoords(0.0,0.0);
	GLdouble params[4];
	glGetDoublev(GL_VIEWPORT,params);
	viewCoords[1] = -viewCoords[1];
	screenCoords[0] = (((float) viewCoords[0]) * ((float)(viewport.w/2))) + (float) params[0] + viewport.w/2;
	screenCoords[1] = (((float) viewCoords[1]) * ((float)(viewport.h/2))) + (float) params[1] + viewport.h/2;
	return screenCoords;
}


//-------------------------------------------------------------------------------
///
void myMouseFunc( int button, int state, int x, int y ) {
    //Convert the pixel coordinates to view coordinates.
    vec2 screenCoords((double) x, (double) y);
    vec2 viewCoords = inverseViewportTransform(screenCoords);
	//Modify the viewCoords to fit the screen and canvas size
	viewCoords[0] = vporigin[0]+viewCoords[0]*vpx;
	viewCoords[1] = vporigin[1]+viewCoords[1]*vpy;
	//save the current position of the cursor
	tempVertex = new Vertex(viewCoords);
	if(!output100) {
		if ( button==GLUT_LEFT_BUTTON && state==GLUT_DOWN ) { 
			cout << "Mouseclick at " << viewCoords[0] << "," << viewCoords[1] << "." << endl;
			//Find the closest vertex to the cursor
			Vertex * closest = polygon->closest(*tempVertex);
			if (groupDrag) {
				cout << "Beginning group drag" << endl;
				picked = showBoundingBox = true;	// turn on the bounding box
			} else if (closest != NULL && distance(viewCoords[0],viewCoords[1],closest->getPos(1.0)[0],closest->getPos(1.0)[1])<=pickingLimit) {
				//If the closest vertex is without our picking bounds
				picked = showBoundingBox = true;				// turn on the bounding box
				pickedGroup.push_back(closest);					// pick up the closest vertex
				cout << "Picked point at " << closest->getPos(1.0)[0] << "," << closest->getPos(1.0)[1] << endl;
			} else if (closest == NULL) {
				// if we did not find a closest, then it's buggy
				cout << "Did not find a closest point" << endl;
			} else if (!groupSelect){
				// otherwise, we did not find a vertex close enough and can use 
				//drag to select multiple vertices
				groupSelect = true; 				// turn on groupSelect
				gsxMin = gsxMax = viewCoords[0];	// Instantiate select box bounds
				gsyMin = gsyMax = viewCoords[1];
				cout << "Distance to point " << closest->getPos(1.0)[0] << "," << closest->getPos(1.0)[1] << " was " << distance(viewCoords[0],viewCoords[1],closest->getPos(1.0)[0],closest->getPos(1.0)[1]) << endl;
			}
		}

		if ( button==GLUT_LEFT_BUTTON && state==GLUT_UP ) {
			
			if (groupSelect) {
				pickedGroup = polygon->findVerticesIn(gsxMin,gsxMax,gsyMin,gsyMax);
				groupSelect = false;
				if (pickedGroup.size() > 0)
					groupDrag = true;
				cout << "Selected " << pickedGroup.size() << "vertices" << endl;
			} else 	if (groupDrag) {
					picked = showBoundingBox = groupSelect = groupDrag = false;
					pickedGroup.clear();
			}
			if (picked) {
				picked = showBoundingBox = false;
				pickedGroup.clear();
			}
		
			delete tempVertex;

		}

		if ( button==GLUT_RIGHT_BUTTON && state==GLUT_DOWN ) {

			showMorph = showBoundingBox = true;
		}
		
		if ( button==GLUT_RIGHT_BUTTON && state==GLUT_UP ) {

			showMorph = showBoundingBox = false;
			t=1.0;
			
		}
	}
	//Force a redraw of the window
	glutPostRedisplay();

}


//-------------------------------------------------------------------------------
/// Called whenever the mouse moves while a button is pressed
void myActiveMotionFunc(int x, int y) {
    //Record the mouse location for drawing crosshairs
	viewport.mousePos = inverseViewportTransform(vec2((double)x,(double)y));
	vec2 mousePos(viewport.mousePos[0]*vpx+vporigin[0],viewport.mousePos[1]*vpy+vporigin[1]);
    if (!output100){
		if (showMorph) {
			if (mousePos[0] < 0.0 && mousePos[0] >= -1.0) 
				sliderPos = (mousePos[0] + 1.0 );
			else if (mousePos[0] > 0.0 && mousePos[0] <= 1.0)
				sliderPos = mousePos[0] + 1.0;
			t = sliderPos / 2.0;
		} else if (groupSelect){
			gsxMin = min(mousePos[0],tempVertex->getPos()[0]);
			gsxMax = max(mousePos[0],tempVertex->getPos()[0]);
			gsyMin = min(mousePos[1],tempVertex->getPos()[1]);
			gsyMax = max(mousePos[1],tempVertex->getPos()[1]);
		} else {
			unsigned int i;
			if (canMoveVertical()) {
				for (i=0;i<pickedGroup.size();i++) {
					vec2 tempEnd = mousePos - tempVertex->getPos() + pickedGroup.at(i)->getPos(1.0);
					vec2 newEnd(pickedGroup.at(i)->getPos(1.0)[0],tempEnd[1]);
					pickedGroup.at(i)->setEndPos(newEnd);
				}
			}
			if (canMoveHorizontal()) {
				for (i=0;i<pickedGroup.size();i++) {
					vec2 tempEnd = mousePos - tempVertex->getPos() + pickedGroup.at(i)->getPos(1.0);
					vec2 newEnd(tempEnd[0],pickedGroup.at(i)->getPos(1.0)[1]);
					pickedGroup.at(i)->setEndPos(newEnd);
				}
			}
			tempVertex->setStartPos(mousePos);
		} 
	}
    //Force a redraw of the window.
    glutPostRedisplay();
}


//-------------------------------------------------------------------------------
/// Called whenever the mouse moves without any buttons pressed.
void myPassiveMotionFunc(int x, int y) {
    //Record the mouse location for drawing crosshairs
	viewport.mousePos = inverseViewportTransform(vec2((double)x,(double)y));
    //Force a redraw of the window.
    glutPostRedisplay();
}


//-------------------------------------------------------------------------------
/// Initialize the environment
int main(int argc,char** argv) {
	//Initialize OpenGL
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);

	//Set up global variables
	viewport.w = 600;
	viewport.h = 600;

	if (argc < 2) {
	    cout << "USAGE: morph poly.obj" << endl;
	    exit(1);
	}

	//Initialize the screen capture class to save BMP captures
	//in the current directory, with the prefix "morph"
	imgSaver = new UCB::ImageSaver("./", "morph");

	//Create OpenGL Window
	glutInitWindowSize(viewport.w,viewport.h);
	glutInitWindowPosition(0,0);
	glutCreateWindow("CS184 Assignment 2 - Richard Nguyen");
	glClearColor(0.3f,0.3f,0.3f,0.0f);
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	glLineStipple(1, (short) 0x5555);	

	//Register event handlers with OpenGL.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(myKeyboardFunc);
	glutMouseFunc(myMouseFunc);
	glutMotionFunc(myActiveMotionFunc);
	glutPassiveMotionFunc(myPassiveMotionFunc);
	
	//And Go!
	glutMainLoop();
}
