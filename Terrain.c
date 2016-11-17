/*

Name:   Damian Angelone
ID:     001408211
macID:  angelodp
E-mail: angelodp@mcmaster.ca

SPECIAL FEATURES THAT ARE IMPLEMENTED:

1) When in colour mode, the program will colour the terrain in regards to differnt heights.
	-This works for both circles/fault algorithm, and drawing wit quad and triangle strips.
		-Water is lowest.
		-Grass is next.
		-Moutain is next.
		-Snow is highest.

2) Implemented the fault algorithm.
	-This can be toggled with the circle's algorithm using the 'c' key.
	-Works with lighting/colour mode, flat/smooth shading, and drawing with triangle/quad strips.

3) Advanced camera / original idea (BONUS).
	-I've wanted to incorporate the mouse into one of my programs for a while now.
	-Rather than following the guidelines for advanced camera, I've redefined my own.
	-When pressing the left/right arrow keys, the terrain will spin around it's center.
		-This looks like the camera is orbitting around the terrain, while looking at the center.
	-Rather than using the up/down keys, I've implemented the mouse instead.
		-When holding a pressing the left button, you have full control of the camera direction.
		-It will reset back to zero upon the release of the button and rotating the terrain (avoid glitching).

CONTROLS:

Use left and right arrow keys to rotate the camera around the grid. 
Use the mouse to control the camera's view.                      
Press 'r' to randomize the grid.                                 
Press 'R' to reset the camera and board.                                   
Press 'c' to switch between Circle and Fault algorithms.         
Press 'f' to switch between glat and smooth shading.             
Press 'w' to cycle through mesh, wireframe, and mesh + wireframe.
Press 'l' to toggle between light and colour mode.
Press 'q' to quit the program.                  
Press 't' to draw terrain with triangles.       
Press 'y' to draw terrain with quads.           
Press 'y' to draw terrain with quads.           
Control light #1 (yellow) with keys 'u' and 'i'.
Control light #2 (red) with keys 'o' and 'p'.

*/

//opengl cross platform includes (final draft)
#  include <stdio.h>
#  include <stdlib.h>
#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif

#include <iostream>
#include <ctime>
#include <cmath>
using namespace std;

int GRID;  //Board dimensions will be size x size.

float HEIGHT_MAP[301][301]; //Keeps track of the heights on the grid.
int wireframe = 0; //Keeps track of the current mode of display (3 modes).
bool shapeModeQuads = true; //Keeps track how display is drawn (triangles or quads).
bool lightingMode = false; //Keeps track of it is colour or lighting mode.
bool gouraud = false; //Keeps track of if it is flat or smooth shading.
bool circleAlg = true; //Keeps track of if circles or fault algorithm is used.
int numberOfHills; //Number of iterations in the algorithms.
float normalVectors[301][301][3]; //Stores the normals for each face.

float maxHeight; //Max height of the board.
float minHeight; //Min height of the board.

/* Eye location */
float eye[3]; //Current camera position in gluLookat( . . . )
float rotat[] = {0, 0}; //Current rotation matrix for the camera.

/* Used for the mouse functions */
float angle = 0.0f; 
float lx = 0.0f, lz = 0.0f;
float deltaAngle = 0.0f;
float deltaAngle2 = 0.0f;
int xOrigin = -1;
int yOrigin = -1;

/* Used to control the 2 light sources */
float cam1;
float cam2;

/* Callback for whe nthe mouse is clicked */
void mouseButton(int button, int state, int x, int y){

	//if the left button is being used
	if(button == GLUT_LEFT_BUTTON){
		//if the left button is being released.
		if(state == GLUT_UP){
			//reset current mouse angle and disallows mouseMouse from running.
			lx = 0;
			lz = 0;
			angle = 0;
			xOrigin = -1;
			yOrigin = -1;
			glutPostRedisplay();
		}
		//if the left button is being pressed.
		else{
			//sets current x,y coordinate of the mouseas the cooridinates to be used for calculation.
			xOrigin = x;
			yOrigin = y;
		}
	}
}

/* Callback for when the mouse is moving */
void mouseMove(int x, int y){

	//if the left button is being pressed (no released).
	if (xOrigin >= 0){
		//calculates the the distance the camera will move when the mouse is moved.
		deltaAngle = (x - xOrigin) * 0.001f;
		deltaAngle2 = (y - yOrigin) * 0.001f;
		lx = sin(angle - deltaAngle) * 70;
		lz = sin(angle - deltaAngle2) * 90;
	}
		glutPostRedisplay();
}

/* Creates the normals for each face */
void createNormals(){

	float v1[3];	//vector 1
	float v2[3];	//vector 2
	float v[3];		//normal vector

	//will run for every vertex in the grid.
	for(int x = 0; x < GRID; x++){
        for(int z = 0; z < GRID; z++){
        
        	//vector 1
        	v1[0] = x;
        	v1[1] = HEIGHT_MAP[x][z];
        	v1[2] = z;

        	//vector 2
        	v2[0] = x + 1;
        	v2[1] = HEIGHT_MAP[x + 1][z];
        	v2[2] = z;

        	//cross-product of the 2 vectors.
        	v[0] = v1[1]*v2[2] - v1[2]*v2[1];	//vx = (v1y * v2z) - (v1z * v2y)
        	v[1] = v1[2]*v2[0] - v1[0]*v2[2];	//vy = (v1z * v2x) - (v1z * v2z)
        	v[2] = v1[0]*v2[1] - v1[1]*v2[0];	//vx = (v1x * v2y) - (v1y * v2x)

        	//normalized length of the cross-product.
        	float normalized = sqrtf(pow(v[0],2) + pow(v[1],2) + pow(v[2],2)); 

        	//normal vector of the current plane.
        	normalVectors[x][z][0] = v[0]/normalized;
        	normalVectors[x][z][1] = v[1]/normalized;
        	normalVectors[x][z][2] = v[2]/normalized; 

        }
    }	
}

/*Gets the designated coloru for the specific height */
void getColour(int x, int z){

	//calculates the scale to base all heights off of.
	float heightColour = maxHeight - minHeight;

	//uses circle algorithm
	if (circleAlg){
		//lighting is off
		if (!lightingMode){
			//calculates the current height in reference to the scale.
			float currHeight = HEIGHT_MAP[x][z]/heightColour;
			//snow colour
			if(currHeight >= 0.7)
				glColor3f(1, HEIGHT_MAP[x][z] / heightColour, 1);
			//moutain colour			
			else if(currHeight >= 0.4)
				glColor3f(1, HEIGHT_MAP[x][z] / heightColour, 0);
			//grass colour
			else if(currHeight >= 0.1)
				glColor3f(0.2, HEIGHT_MAP[x][z] / heightColour, 0);
			//water colour
			else 
				glColor3f(0, HEIGHT_MAP[x][z] / heightColour, 1);
		}
		//lighting is on (default colour).
		else if (lightingMode){
			glColor3f(1, HEIGHT_MAP[x][z] / heightColour, 0);
		}
	}
	//uses fault algorithm
	else{
		//lighting is off
		if (!lightingMode){
			//calculates the current height in reference to the scale.
			float currHeight = HEIGHT_MAP[x][z]/heightColour;
			//snow colour
			if(currHeight >= 0.9)
				glColor3f(1, HEIGHT_MAP[x][z] / heightColour, 1);
			//moutain colour			
			else if(currHeight >= 0.6)
				glColor3f(1, HEIGHT_MAP[x][z] / heightColour, 0);
			//grass colour
			else if(currHeight >= 0)
				glColor3f(0.2, (HEIGHT_MAP[x][z] / heightColour) + 0.2, 0);
			//water colour
			else 
				glColor3f(0, (1 - abs(HEIGHT_MAP[x][z] / heightColour)) , 1);
		}
		//lighting is on (default colour).
		else if (lightingMode){
			glColor3f(1, HEIGHT_MAP[x][z] / heightColour, 0);
		}
	}
}

/*Draws the mesh */
void createMesh(){

	//draw with quads
	if (shapeModeQuads){
		//draws vertices in clockwise direction.
		glFrontFace(GL_CW);
		//will run for every vertex in the grid.
		for(int x = 0; x < GRID - 1; x++){
			for(int z = 0; z < GRID - 1; z++){			
				//creates the normal vectors.		
				glNormal3f(normalVectors[x][z][0], normalVectors[x][z][1], normalVectors[x][z][2]);
				//draws using quad strips.
				glBegin(GL_QUAD_STRIP);
				getColour(x+1, z);
				glVertex3f(x + 1, HEIGHT_MAP[x + 1][z]    , z    );
				getColour(x+1, z+1);
				glVertex3f(x + 1, HEIGHT_MAP[x + 1][z + 1], z + 1);
				getColour(x, z);
				glVertex3f(x    , HEIGHT_MAP[x][z]        , z    );
				getColour(x, z+1);
				glVertex3f(x    , HEIGHT_MAP[x][z + 1]    , z + 1);
				glEnd();
			}
		}
	}
	//draw with triangles
	else{
		//draws vertices in clockwise direction.
		glFrontFace(GL_CW);
		//draws using triangle strips.
		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 1; x < GRID; x++){
			if (x % 2 == 0){
				//draws vertices of odd rows (moving forward).
				for(int z = 0; z < GRID; z++){
					//creates the normal vectors.	
					glNormal3f(normalVectors[x][z][0], normalVectors[x][z][1], normalVectors[x][z][2]);
					getColour(x-1,z);
					glVertex3f(x - 1, HEIGHT_MAP[x-1][z], z);
					getColour(x,z);
					glVertex3f(x, HEIGHT_MAP[x][z], z);
				}
			}
			else{
				//draws vertices of even rows (moving backwards).
				for(int z = GRID-1; z > -1; z--){
					//creates the normal vectors.	
					glNormal3f(normalVectors[x][z][0], normalVectors[x][z][1], normalVectors[x][z][2]);
					getColour(x,z);
					glVertex3f(x, HEIGHT_MAP[x][z], z);
					getColour(x-1,z);
					glVertex3f(x-1, HEIGHT_MAP[x-1][z], z);
				}
			}
		}
		glEnd();
	}
}	

/* Draws the wireframe */
void createGrid(void){
		
	//sets colours of grid depending of the wireframe mode.	
	if(wireframe == 2)
		glColor3f(0,0,0);
	if(wireframe == 1)
		glColor3f(1,1,0);

	//Draws grid with quads.
	if(shapeModeQuads){
		//draw every vertex in the grid, in a pair at a time (rows).		
		for(int z = 0; z < GRID; z++){	
			for(int x = 1; x < GRID + 1; x++){
				//begins drawing with the lines.
				glBegin(GL_LINES);
					//draws the previous point.
					glVertex3f(x-1, HEIGHT_MAP[x-1][z], z);
					//draws the current point.
					glVertex3f(x, HEIGHT_MAP[x][z], z);		
				glEnd();
			}	
		}
		//draw every vertex in the grid, in a pair at a time (collumns).
		for(int x = 0; x < GRID + 1; x++){
			for(int z = 1; z < GRID; z++){
				//begins drawing with the lines.	
				glBegin(GL_LINES);
					//draws the previous point.
					glVertex3f(x, HEIGHT_MAP[x][z-1], z-1);
					//draws the current point.
					glVertex3f(x, HEIGHT_MAP[x][z], z);		
				glEnd();
			}	
		} 
	}
	//draws the grid with triangles.
	else{
		//begins drawing with lines.
		glBegin(GL_LINES);
		//draw every vertex in the grid, in a pair at a time (rows).	
		for(int x = 0; x < GRID + 1; x++){
			for(int z = 1; z < GRID; z++){
				glVertex3f(x,HEIGHT_MAP[x][z],z);
				glVertex3f(x,HEIGHT_MAP[x][z-1],z-1);
			}
		}
		//draw every vertex in the grid, in a pair at a time (collumns).	
		for(int x = 0; x < GRID + 1; x++){
			for(int z = 1; z < GRID; z++){
				glVertex3f(x,HEIGHT_MAP[x][z],z);
				glVertex3f(x-1,HEIGHT_MAP[x-1][z],z);
			}
		}
		//draw every vertex in the grid, in a pair at a time (diagonals).	
		for(int x = 0; x < GRID + 1; x++){
			for(int z = 1; z < GRID; z++){
				glVertex3f(x,HEIGHT_MAP[x][z],z);
				glVertex3f(x+1,HEIGHT_MAP[x+1][z+1],z+1);
			}
		}
		glEnd();
	}
}

/* Used for creating heightmap using the fault algorithm */
void faultAlgorithm(int iterations){
	
	//will run for how many hills was specified by the user. 
	for(int i = 0; i < iterations; i++){
		float v = (rand());	//random value
		float a = sin(v);	//coefficient of x
		float b = cos(v);	//coefficient of y
		float d = sqrtf(pow(GRID, 2) + pow(GRID, 2)); //max distance the fault line can cover
		float c = (rand() % 1000 / 1000.0)*d - d/2; //random number between -d/2 and d/2.

		//will run for every vertex in the grid.
		for(int x = 0; x < GRID; x++){
            for(int z = 0; z < GRID; z++){
           		//if the point lies on the right of the line.
            	if((a*x + b*z - c) > 0)
            		HEIGHT_MAP[x][z] += 0.3;
            	//if the point lies on the left of the line.
            	else
            		HEIGHT_MAP[x][z] -= 0.3;
            }
        }
	}
	//creates the normal lines.
	createNormals();

	maxHeight = HEIGHT_MAP[0][0];
	minHeight = HEIGHT_MAP[0][0];
	//will run for every vertex in the grid.
	for(int x = 0; x < GRID; x++){
       for(int z = 0; z < GRID; z++){
       		//will find the max height.
       		if(HEIGHT_MAP[x][z] > maxHeight)
       			maxHeight = HEIGHT_MAP[x][z];
       		//will find the min height.
       		if(HEIGHT_MAP[x][z] < minHeight)
       			minHeight = HEIGHT_MAP[x][z];
        }
    }
}

/* Used for creating heightmap using the circle's algorithm */
void createTerrain(int iterations){

	//will run for how many hills was specified by the user. 
	for(int i = 0; i < iterations; i++){

		int center_X = rand() % GRID; 				//GRID (x) midpoint of circle (1-gridLength)                      
	    int center_Z = rand() % GRID;				//GRID (z) midpoint of circle (1-gridWidth)
	    float terrainCircleSize = 20; 				//random radius of circle (1-5)
	    int randomHeight = (rand() % 5) + 1;		//random height for slope (1-5)   
	    
	    //will run for every vertex in the grid.
        for(int x = 0; x < GRID; x++){
            for(int z = 0; z < GRID; z++){

				float distanceFromX = x - center_X;
                float distanceFromZ = z - center_Z;
                float totalDistance = sqrtf((distanceFromX * distanceFromX) + (distanceFromZ * distanceFromZ));
                float pd = (totalDistance * 2) / terrainCircleSize;

                if (fabs(pd) <= 1.0){
             	
                	HEIGHT_MAP[x][z] += randomHeight / 2.0 + cos(pd * 3.14) * randomHeight / 2.0;
                }
            }
        } 
	}
	//create normals for each plane.
	createNormals();

	maxHeight = HEIGHT_MAP[0][0];
	minHeight = HEIGHT_MAP[0][0];
	//will run for every vertex in the grid.
    for(int x = 0; x < GRID; x++){
        for(int z = 0; z < GRID; z++){
       		//find the max height.
       		if(HEIGHT_MAP[x][z] > maxHeight)
       			maxHeight = HEIGHT_MAP[x][z];
       		//find the min height.
       		if(HEIGHT_MAP[x][z] < minHeight)
       			minHeight = HEIGHT_MAP[x][z];
        }
    }
}

/* Used for adjusting the lights */
void lightingSettings(){

	glEnable(GL_LIGHTING); 	//enables lighting 
	glEnable(GL_LIGHT0);	//first lightbulb
	glEnable(GL_LIGHT1);	//second lightbulb
	
	//position of the first camera.
	float pos1[4] = {0, 20, cam1, 1};
	//lighting settings of the first camera.
	float amb[4] = {0.5, 0.5, 0, 1};  
	float dif[4] = {0.8, 0.8, 0, 1};
	float spc[4] = {1, 1, 1, 1};

	//upload first light data to gpu.
	glLightfv(GL_LIGHT0, GL_POSITION, pos1);

	//set lighting parameters of light 1.
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	glLightfv(GL_LIGHT0, GL_SPECULAR, spc);

	
	float pos2[4] = {cam2, 20, 0, 1};
	//lighting settings of the second camera.
	float amb2[4] = {1, 0, 1, 1};  
	float dif2[4] = {1, 0, 1, 1};
	float spc2[4] = {1, 1, 1, 1};
	
	//upload data light data to gpu.
	glLightfv(GL_LIGHT1, GL_POSITION, pos2);

	//set lighting parameters of light 1.
	glLightfv(GL_LIGHT1, GL_DIFFUSE, dif2);
	glLightfv(GL_LIGHT1, GL_AMBIENT, amb2);
	glLightfv(GL_LIGHT1, GL_SPECULAR, spc2);
	
	//set material of the terrain.
	float m_dif[] = {1, 1, 0, 0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif);
}

/* Displays UI to user */
void display(void){

	//Clears the board upon (re)display
	glClearColor(0,0,0,0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Sets up the perspective matrix (viewing/camera)
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();
	//sets up the camera position and view.
	gluLookAt(eye[0], eye[1], eye[2], GRID/2 + lx, 0, GRID/2 + lz, 0, 1, 0);
	//disables lighting.
	glDisable(GL_LIGHTING);
	//turn lighting on.
	if (lightingMode){
		lightingSettings();
	}
	//toggle between two shadin types.
	glShadeModel(!gouraud ? GL_FLAT : GL_SMOOTH);

	//rotates the terrain around it's centre.
	glPushMatrix();
	glTranslatef(GRID/2, 0, GRID/2);
	glRotatef(rotat[1], 0, 1, 0);
	glTranslatef(-GRID/2, 0, -GRID/2);

	//draws mesh.
	if(wireframe == 0)
		createMesh();
	//draws grid.
	else if(wireframe == 1)
		createGrid();
	//draws mesh and grid.
	else if(wireframe == 2){
		createMesh();
		createGrid();
	}
	glPopMatrix();

	//Used for double-buffering
	glutSwapBuffers();
}

/* Clears all values form the array */
void clearArray(){

	//resets the height map back to default (zero)
	for(int x = 0; x < GRID; x++){
        for(int z = 0; z < GRID; z++){
        
        	HEIGHT_MAP[x][z] = 0.0;
        }
    }
}

/* Randomizes the board */
void randomize(){

	//clears height array.
	clearArray();
	//draws the random terrain, depending on what mode is chosen.
	if(circleAlg)
		createTerrain(numberOfHills);
	else
		faultAlgorithm(numberOfHills);
	glutPostRedisplay();
}

/* Resets camera and board settins */
void reset(){

	//resets camera views
	eye[0] = GRID/2;
	eye[1] = 45;
	eye[2] = -GRID/2;
	lx = 0.0;
	lz = 0.0;
	//resets baord settings.
	wireframe = 0;
	gouraud = false;
	lightingMode = false;
	shapeModeQuads = true;
	printf("Camera reset!");
	glutPostRedisplay();
}

/* Gives functionality to certain keyboard inputs */
void keyboard(unsigned char key, int xIn, int yIn){

	switch (key){
		case 'Q':
		case 'q':
		case 27:
			exit(0);
			break;
		case 'r':
			randomize();
			printf("Board randomized!\n\n");
			break;
		case 'R':
			reset();
			printf("Camera reset!\n\n");
			break;
		case 'f':
		case 'F':
			gouraud = !gouraud;
			glutPostRedisplay();
			if(gouraud){
				printf("Shading changed to Gouraud!\n\n");
			}
			else{
				printf("Shading changed to flat!\n\n");
			}
			break;
		case 'w':
		case 'W':
			if(wireframe == 1){
				wireframe = 2;
				printf("Mesh and grid displayed!\n\n");
			}
			else if(wireframe == 0){
				wireframe = 1;
				printf("Grid displayed!\n\n");
			}
			else if(wireframe == 2){
				wireframe = 0;
				printf("Mesh displayed!\n\n");
			}
			glutPostRedisplay();
			break;
		case 'c':
		case 'C':
			circleAlg = !circleAlg;
			clearArray();
			if(circleAlg){
				createTerrain(numberOfHills);
				printf("Circles Algorithm used!\n\n");
			}
			else{
				faultAlgorithm(numberOfHills);
				printf("Fault Algorithm used!\n\n");
			}
			glutPostRedisplay();
			break;
		case 'l':
		case 'L':
			lightingMode = !lightingMode;
			glutPostRedisplay();
			if(lightingMode){
				printf("Lighting mode!\n\n");
			}
			else{
				printf("Colour mode!\n\n");
			}
			break;
		case 't':
			shapeModeQuads = false;
			glutPostRedisplay();
			printf("Drawing with triangles!\n\n");
			break;
		case 'y':
			shapeModeQuads = true;
			glutPostRedisplay();
			printf("Drawing with quads!\n\n");
			break;

		case 'u':
			if(cam1 < GRID && lightingMode)
				cam1 += 1;
			else
				printf("Light source can't go any further, please go the other direction!\n\n");
			glutPostRedisplay();
			break;
		case 'i':
			if(cam1 > 0  && lightingMode)
				cam1 -= 1;
			else
				printf("Light source can't go any further, please go the other direction!\n\n");
			glutPostRedisplay();
			break;

		case 'o':
			if(cam2 < GRID && lightingMode)
				cam2 += 1;
			else
				printf("Light source can't go any further, please go the other direction!\n\n");
			glutPostRedisplay();
		break;
		case 'p':
			if(cam2 > 0 && lightingMode)
				cam2 -= 1;
			else
				printf("Light source can't go any further, please go the other direction!\n\n");
			glutPostRedisplay();
		break;
	}
}

/* Camera controls */
void special(int key, int x, int y){
	
	switch(key)
	{
	case GLUT_KEY_LEFT:
		lx = 0;
		lz = 0; 
		rotat[1] += 3;
	break; 
	case GLUT_KEY_RIGHT:
		lx = 0;
		lz = 0; 
		rotat[1] -= 3;
	break; 	
    }
	glutPostRedisplay();
}

/* Initializes board/camera/colours/sizes/etc. */
void initDefaults(void){
	
	cout << "***********************************************************************" << endl;
	cout << "* Use left and right arrow keys to rotate the camera around the grid. *" << endl;
	cout << "* Use the mouse to control the camera's view.                         *" << endl;
	cout << "* Press 'r' to randomize the grid.                                    *" << endl;
	cout << "* Press 'R' to reset the camera and board settings.                   *" << endl;
	cout << "* Press 'c' to switch between Circle and Fault algorithms.            *" << endl;
	cout << "* Press 'f' to switch between glat and smooth shading.                *" << endl;
	cout << "* Press 'w' to cycle through mesh, wireframe, and mesh + wireframe.   *" << endl;
	cout << "* Press 'l' to toggle between light and colour mode.                  *" << endl;
	cout << "* Press 'q' to quit the program.                                      *" << endl;
	cout << "* Press 't' to draw terrain with triangles.                           *" << endl;
	cout << "* Press 'y' to draw terrain with quads.                               *" << endl;
	cout << "* Press 'y' to draw terrain with quads.                               *" << endl;
	cout << "* Control light #1 (yellow) with keys 'u' and 'i'.                    *" << endl;
	cout << "* Control light #2 (red) with keys 'o' and 'p'.                       *" << endl;
	cout << "***********************************************************************" << endl;

	glClearColor(0, 0, 0, 0);
	glColor3f(1, 1, 1);
    glMatrixMode(GL_PROJECTION);
	gluPerspective(45,1,1,1000);
	glFrustum(10, -10, 10, -10, 0, 100);
	numberOfHills = GRID*2;

	cam1 = GRID;
	cam2 = GRID;

	eye[0] = GRID/2;
	eye[1] = 45;
	eye[2] = -GRID/2;
}

/* main function - program entry point */
int main(int argc, char** argv){
	
	//Prompts the suer to choose the size of the grid.
	while (GRID < 50 || GRID > 300){
		cout << "Please type in how many rows/columns you would like, between 50 and 300:" << endl;
		scanf("%d", &GRID);
	}
	
	glutInit(&argc, argv);		    	//starts up GLUT
	glutInitWindowSize(600, 600);   	//sets window size.
	glutInitWindowPosition(200,200);	//sets window position
	glutCreateWindow("Terrain");		//creates the window
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutDisplayFunc(display);	//registers "display" as the display callback function.
	glutKeyboardFunc(keyboard); //registers "keyboard" as the keyboard callback function.
	glutSpecialFunc(special);   //registers "special" as the special callback function.
	
	glutMouseFunc(mouseButton); //registers "mouseButtons" as the mouse callback function.
	glutMotionFunc(mouseMove);  //registers "mouseMove" as the motion callback function.

	initDefaults(); //Initializes the defautl values.

	//Enables front face culling (only draws faces that are visible).
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	//prevents transparent terrain.
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);	

	srand(time(NULL)); 	//Guarantee rand() to produce a succession of random numbers.
	createTerrain(numberOfHills);

	glutMainLoop();	//starts the event loop

	return(0);	//return may not be necessary on all compilers
}