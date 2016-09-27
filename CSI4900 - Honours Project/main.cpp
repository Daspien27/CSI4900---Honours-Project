#define NOMINMAX
#include <cstdlib>
#include <cmath>
#include <bitset>
#include <cassert>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <GL/glew.h>
#include <GL/glut.h>
#include <stack>

// glm types
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
// matrix manipulation
#include <glm/gtc/matrix_transform.hpp>
// value_ptr
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "shader.h"

/** Global variables */

using namespace std;

/*
* Helper structure for the array of vertices
*/



/*
* Helper structure holding the locations of the uniforms to transform matrices
*/
struct Transformations {
	GLint locM;
	GLint locV;
	GLint locP;
	Transformations() : locM(-1), locV(-1), locP(-1) {}
};
Transformations g_tfm;

/*
* Helper structure holding the locations of the attributes
*/
struct Attributes {

	GLint locPos;
	GLint locNorm;
	GLint locColor;
	GLint locTexture;
	Attributes() : locPos(-1), locNorm(-1), locColor(-1), locTexture(-1) {}
};
Attributes g_attrib;

/*
*  Window dimensions
*/
struct WindowSize {
	GLfloat d_near;
	GLfloat d_far;
	GLint d_widthPixel;
	GLfloat d_width;
	GLint d_heightPixel;
	GLfloat d_height;
	bool d_perspective;

	WindowSize() : d_near(1.0f), d_far(300.0f),d_perspective(false),

		d_widthPixel(512), d_width(30.0f),
		d_heightPixel(512), d_height(30.0f)
	{}
};
WindowSize g_winSize;

GLuint g_program;
GLuint* g_bufferObjects;




/**
* Sloppy camera code
*/
bool left_drag = false;
int last_x = 0, last_y = 0;
GLfloat vertical_rotation = 0.0f, horizontal_rotation = 0.0f;
const glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f); //Camera is always looking at the origin
glm::vec3 camera_eye = glm::vec3(0.0f, 0.0f, -80.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camera_right;
glm::vec3 target_to_camera;

void updateCameraVectors() {
	// Create the new camera up and right vectors
	camera_right = glm::normalize(glm::cross(camera_eye, camera_up));
	camera_up = glm::normalize(glm::cross(camera_right, camera_eye));

	// Rotate the eye accordingly
	camera_eye = glm::rotate(camera_eye, horizontal_rotation, camera_up);
	camera_eye = glm::rotate(camera_eye, vertical_rotation, camera_right);
}

/********************************/
int currentHash = 0b10001000;


glm::vec3 interpolate(GLfloat isolevel, glm::vec3 p1, glm::vec3 p2, GLfloat valp1, GLfloat valp2) {
	//Borrowed from http://paulbourke.net/geometry/polygonise/
	glm::vec3 p;

	if (glm::abs(isolevel - valp1) < 0.00001)return(p1);
	if (glm::abs(isolevel - valp2) < 0.00001)return(p2);
	if (glm::abs(valp1 - valp2) < 0.00001)  return(p1);
	
	GLfloat mu = (isolevel - valp1) / (valp2 - valp1);
	p.x = p1.x + mu * (p2.x - p1.x);
	p.y = p1.y + mu * (p2.y - p1.y);
	p.z = p1.z + mu * (p2.z - p1.z);

	return(p);

	//return (p1+p2)/2.0f;
}

void march(glm::vec3 center) {
	//Check to ensure I have correct centers
	//cout << center.x << " " << center.y << " "  << center.z << endl;

	glm::vec3 vertices[] = { glm::vec3(-1.0f,-1.0f,-1.0f) + center,
								glm::vec3(-1.0f,-1.0f,1.0f) + center,
								glm::vec3(1.0f,-1.0f,1.0f) + center,
								glm::vec3(1.0f,-1.0f,-1.0f) + center,
								glm::vec3(-1.0f,1.0f,-1.0f) + center,
								glm::vec3(-1.0f,1.0f,1.0f) + center,
								glm::vec3(1.0f,1.0f,1.0f) + center,
								glm::vec3(1.0f,1.0f,-1.0f) + center
	};

	int numVerts = sizeof(vertices) / sizeof(*vertices);

	GLfloat isolevel = 100.0f;
	int vertHash = 0; //This is an  integer to track the vertices contained in the isosurface
	
	for (int i = 0; i < numVerts; i++) {
		//If vertex is within the isosurface
		if (glm::dot(vertices[i], vertices[i]) <= isolevel) {
			vertHash = vertHash | 1 << i;
		}
	}
	
	//Cube edges
	int edges[] = {
						0b00011010,
						0b00100101,
						0b01001010,
						0b10000101,
						0b10100001,
						0b01010010,
						0b10100100,
						0b01011000,
	};
	

	//Cube faces
	int faces[] = {
						0b00001111,
						0b00110011,
						0b01100110,
						0b11001100,
						0b10011001,
						0b11110000

	};
	

	//Do the following when the cube is neither fully in or out of the sphere
	if (vertHash != 0x00 && vertHash != 0xFF) {
		//cout << bitset<8>(vertHash) << endl;


		//Find each plane of intersection on cube
		
		
		
		//"Bubble merge" this will be fairly inefficient ~ for now
		bool bubbled = false;
		vector<int> planes = vector<int>();
		for (int i = 0; i < 8; i++) {
			
			if ((vertHash & 1<<i) != 0) {
				//cout << "\t*" << bitset<8>(vertHash & (1 << i)) << "*" << (vertHash & (1 << i)) << endl;
				planes.push_back((edges[i] & vertHash) | (1<<i)); //each vertice that is contained make a list of its neighbours

			}
		}

		/*
		for (int i = 0; i < planes.size(); i++) {

			cout << "\t" << bitset<8>(planes[i]) << endl;
		}
		cout << "--------" << endl;
		*/
		while (!bubbled) {
			vector<int> mergedPlanes = vector<int>();
			
			bubbled = true;

			while (!planes.empty()) {
				int add = planes.back();
				planes.pop_back();

				//Take the list of neighbours that are not linearly independent  (within the isosurface) and merge them
				bool shared = false;
				
				for (std::vector<int>::iterator it = mergedPlanes.begin(); it != mergedPlanes.end(); ++it) {
					int test = (add & *it);
					if ((add & *it) != 0) {
						//cout << "\t" << bitset<8>(*it) << "*" << endl;
						*it = add | *it;
						shared = true;
						break;
						//cout << *it << endl;
					}
				}

				if (shared == false) {
					mergedPlanes.push_back(add);
				}
				else {
					bubbled = false;
				}

			}

			planes = mergedPlanes;
		}

		/*
		for (int i = 0; i < planes.size(); i++) {

			cout << "\t" << bitset<8>(planes[i]) <<endl;
		}
		*/
		


		//For each plane of intersection render the plane

		for (std::vector<int>::iterator it = planes.begin(); it != planes.end(); ++it) {
			//Render each midpoint: use GL_LINE_LOOP, or GL_TRIANGLE_STRIP (or GL_POLYGON)

			vector<int> crossedges = vector<int>();
			
			for (int i = 0; i < 8; i++) {
				//Iterate for each vertice contained under the plane
				if ((*it & 1 << i) != 0) {
					int edgeHash = ~*it & edges[i];


					//cout << "\t" << bitset<8>(~vertHash) << " & " << bitset<8>(edges[i]) << " = " << bitset<8>((~vertHash) & (edges[i])) << endl;

					for (int e = 0; e < 8; e++) {
						//Check the edges with one vertice in and one vertice out in the cube
						if (edgeHash >> e & 1) {
							

							
							crossedges.push_back(1<<i | 1<<e);

						}

					}
				}

				//cout << endl;
			}
			
			//for each edge find a pair of edges sharing a face (only need one direction)
			vector<std::pair<int,int>> triangle = vector<pair<int,int>>();

			for (int i = 0; i < crossedges.size() - 1; i++) {
				for (int j = i + 1; j < crossedges.size(); j++) {

					//For each face check if the edges share the face
					for (int f = 0; f < 6; f++) {

						int tf = crossedges[i] | crossedges[j];
						if ((faces[f] | tf) == faces[f]) {
							//They are on the same face
							triangle.push_back(std::make_pair(crossedges[i], crossedges[j]));
						}

					}

				}

			}


			glm::vec3 centroid = glm::vec3(0.0f);

			for (int i = 0; i < crossedges.size(); i++) {

				glm::vec3 a = glm::vec3(0.0f);
				glm::vec3 b = glm::vec3(0.0f);

				bool pairsecond = false;
				for (int v = 0; v < 8; v++) {
					if (!pairsecond) {
						if ((crossedges[i] & 1<<v) != 0) {
							a = vertices[v];
							pairsecond = true;
						}
					}
					else {
						if ((crossedges[i] & 1<<v) != 0) {
							b = vertices[v];
						}

					}
					
				}

				centroid += interpolate(isolevel, a, b, glm::dot(a,a), glm::dot(b,b));
			}

			centroid /= (GLfloat)crossedges.size();

			glBegin(GL_TRIANGLES); //USE GL_TRIANGLES or GL_LINE_LOOP

			for (int i = 0; i < triangle.size(); i++) {

				glVertex3fv(glm::value_ptr(centroid));
				
				glm::vec3 a[2] = { glm::vec3(0.0f) , glm::vec3(0.0f)};
				glm::vec3 b[2] = { glm::vec3(0.0f) , glm::vec3(0.0f) };

				bool secondpair[2] = { false, false };
				for (int v = 0; v < 8; v++) {
					
					if (!secondpair[0]) {
						if ((triangle[i].first & 1 << v) != 0) {
							a[0] = vertices[v];
							secondpair[0] = true;
						}
					}
					else {
						if ((triangle[i].first & 1 << v) != 0) {
							b[0] = vertices[v];
						}
					}
					

					if (!secondpair[1]) {
						if ((triangle[i].second & 1 << v) != 0) {
							a[1] = vertices[v];
							secondpair[1] = true;
						}
					}
					else {
						if ((triangle[i].second & 1 << v) != 0) {
							b[1] = vertices[v];
						}
					}
					
				}
				
				glm::vec3 interPair[] = { interpolate(isolevel, a[0], b[0], glm::dot(a[0],a[0]), glm::dot(b[0],b[0])),
										  interpolate(isolevel, a[1], b[1], glm::dot(a[1],a[1]), glm::dot(b[1],b[1])) };
				

				glVertex3fv(glm::value_ptr(interPair[0]));
				glVertex3fv(glm::value_ptr(interPair[1]));


			}

			glEnd();
		}

		

	}

	



}

void marchingCubes() {

	//Direction vectors of a cube
	//This is an array s.t. you can tile 3d with a linear combination of these centers
	glm::vec3 d[] = { glm::vec3(2.0f,0.0f,0.0f) ,glm::vec3(0.0f,2.0f,0.0f) ,glm::vec3(0.0f,0.0f,2.0f) };
	
	

	int size = 6;
	for (int i = -0; i < size; i++)
		for (int j = -0; j < size; j++)
			for (int k = -0; k < size; k++) {
				
				glm::vec3 center = (GLfloat)(i)*d[0] + (GLfloat)(j)* d[1] + (GLfloat)(k)* d[2];
				
				/*
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, center);
				glUniformMatrix4fv(g_tfm.locM, 1, GL_FALSE, glm::value_ptr(model));
				glutWireCube(2.0f);
				*/
				//March Cubes

				march(center);

			}
				
				
			




}




/*********************************/
/**
* Display routine - vertex array object
*/
void display(void) {
	// Clear the window
	glClear(GL_COLOR_BUFFER_BIT);
	// Use glm to emulate fixed function pipeline
	// Load identity
	

	//Direct camera using our coordinate's basis
	if (left_drag == true) updateCameraVectors(); //Don't update camera unless the camera is being moved
	glm::mat4 View = glm::lookAt(camera_eye, camera_target, camera_up);
	glm::mat4 Model = glm::mat4(1.0f);


	// Update uniform for this drawing
	
	glUniformMatrix4fv(g_tfm.locV, 1, GL_FALSE, glm::value_ptr(View));
	glUniformMatrix4fv(g_tfm.locM, 1, GL_FALSE, glm::value_ptr(Model));


	//Call to marching draw command here
	/**TODO
		Allow generalization of object (not just cubes)
		Decide when to choose isosurface to be rendered
	*/

	marchingCubes();


	glutWireSphere(10.0f,20,20);
	//glutSolidSphere(15.0f, 20, 20);

	// swap buffers
	glutSwapBuffers();
}


/**
* Idle routine - rotate basic shape when we have
* nothing else to do
*/
void idleFunc() {

}

/**
* Keyboard routine - handle keyboard input
*/
void keyboardFunc(unsigned char _key, int _x, int _y) {
	switch (_key) {
	case 27:
	case 'q':

		exit(0);
		break;

	case 'r':
	case 'R':

		// Reset camera to be on z axis looking at origin
		camera_right = glm::vec3(1.0f, 0.0f, 0.0f);
		camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
		camera_eye = glm::vec3(0.0f, 0.0f, -5.0f);
		horizontal_rotation = 0;
		vertical_rotation = 0;
		break;
	case 'i':
	case 'I':
		currentHash = (currentHash + 1) % 0xFF;
		cout << "CurrentHash: " << currentHash << " (" << bitset<8>(currentHash) << ")" << endl;
	default:

		break;
	}
	glutPostRedisplay();
	return;
}

/**
* Special key routine
*/
void specialKeyboardFunc(int _key, int _x, int _y) {
	
	glutPostRedisplay();
	return;
}

// mouse movement callback
void mouseFunc(int button, int state, int x, int y) {

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		left_drag = true;

		last_x = x;
		last_y = y;

		glutPostRedisplay();
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		left_drag = false;

		glutPostRedisplay();
	}


}

// mouse button callback
void motionFunc(int x, int y) {
	if (left_drag) {
		horizontal_rotation = (float)(last_x - x) * 0.01f;
		vertical_rotation = (float)(y - last_y) * 0.01f;

		last_x = x;
		last_y = y;

		glutPostRedisplay();
	}

}


/**
* OpenGL reshape function - main window
*/
void reshapeFunc(GLsizei _width, GLsizei _height) {
	errorOut();
	GLfloat minDim = std::min(g_winSize.d_width, g_winSize.d_height);
	glm::mat4 Projection;
	
	// adjust the view volume to the correct aspect ratio
	if (_width > _height) {
		g_winSize.d_width = minDim  * (GLfloat)_width / (GLfloat)_height;
		g_winSize.d_height = minDim;
	}
	else {
		g_winSize.d_width = minDim;
		g_winSize.d_height = minDim * (GLfloat)_height / (GLfloat)_width;
	}
	if (g_winSize.d_perspective) {
		Projection = glm::frustum(-g_winSize.d_width / 2.0f, g_winSize.d_width / 2.0f,
			-g_winSize.d_height / 2.0f, g_winSize.d_height / 2.0f,
			g_winSize.d_near, g_winSize.d_far);
	}
	else {
		Projection = glm::ortho(-g_winSize.d_width / 2.0f, g_winSize.d_width / 2.0f,
			-g_winSize.d_height / 2.0f, g_winSize.d_height / 2.0f,
			g_winSize.d_near, g_winSize.d_far);
	}

	glUniformMatrix4fv(g_tfm.locP, 1, GL_FALSE, glm::value_ptr(Projection));

	// Wait for updating projection in display routine
	g_winSize.d_widthPixel = _width;
	g_winSize.d_heightPixel = _height;
	// reshape our viewport
	glViewport(0, 0,
		g_winSize.d_widthPixel,
		g_winSize.d_heightPixel);
	errorOut();
}


/**
* OpenGL initialization - set the state machine
*/
void init(void) {
	// darkgray background
	glClearColor(0.4f, 0.4f, 0.4f, 0.0f);
	// Point size to something visible
	glPointSize(2.0f);

	// Make sure that our shaders run
	int major, minor;
	getGlVersion(major, minor);
	cerr << "Running OpenGL " << major << "." << minor << endl;
	if (major < 3 || (major == 3 && minor<3)) {
		cerr << "No OpenGL 3.3 or higher" << endl;
		exit(-1);
	}

	
	// Generate a VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	g_bufferObjects = new GLuint[1];

	glGenBuffers(1, g_bufferObjects);

	GLint size = 1; //Dummy Size
	glm::vec4 vert = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); //Dummy array
	
	glBindBuffer(GL_ARRAY_BUFFER, g_bufferObjects[0]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec4) * size,
		&vert, GL_STATIC_DRAW);
	
	// pointer into the array of vertices which is now in the VAO
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	
	
	// Load shaders
	vector<GLuint> sHandles;
	GLuint handle;
	Shader shader;
	if (!shader.load("planet.vs", GL_VERTEX_SHADER)) {
		shader.installShader(handle, GL_VERTEX_SHADER);
		Shader::compile(handle);
		sHandles.push_back(handle);
	}
	if (!shader.load("planet.fs", GL_FRAGMENT_SHADER)) {
		shader.installShader(handle, GL_FRAGMENT_SHADER);
		Shader::compile(handle);
		sHandles.push_back(handle);
	}
	cerr << "No of handles: " << sHandles.size() << endl;
	Shader::installProgram(sHandles, g_program);
	

	// Activate program in order to be able to set uniforms 
	glUseProgram(g_program);
	
	// find the locations of our uniforms and store them in a global structure for later access
	g_tfm.locM = glGetUniformLocation(g_program, "ModelMatrix");
	g_tfm.locV = glGetUniformLocation(g_program, "ViewMatrix");
	g_tfm.locP = glGetUniformLocation(g_program, "ProjectionMatrix");
	

	// set the projection matrix with a uniform
	glm::mat4 Projection = glm::ortho(-g_winSize.d_width / 2.0f, g_winSize.d_width / 2.0f,
		-g_winSize.d_height / 2.0f, g_winSize.d_height / 2.0f);
	glUniformMatrix4fv(g_tfm.locP, 1, GL_FALSE, glm::value_ptr(Projection));
	

	return;
}


/**
* Main for a simple GLUT application
*/
int main(int argc, char** argv) {
	// Pass the program name on
	glutInit(&argc, argv);
	// Set the glut state
	// Double buffering and RGB color 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	// Set-up the glut OpenGL window
	// glut's coordinate system has origin at top-left corner
	// ( column, row )
	glutInitWindowPosition(0, 0);
	// Window w/h according to g_winSize
	glutInitWindowSize(g_winSize.d_widthPixel, g_winSize.d_heightPixel);
	glutCreateWindow("CSI4900 - Honours Project");
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		/* Problem: glewInit failed, something is seriously wrong. */
		cerr << "Error: " << glewGetErrorString(err) << endl;
		return -1;
	}
	cerr << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// Initialize OpenGL state
	init();

	// Set up callback functions for key presses
	glutKeyboardFunc(keyboardFunc); // Handles ascii symbols
	glutSpecialFunc(specialKeyboardFunc); // Handles function keys

	
	glutMouseFunc(mouseFunc);
	glutMotionFunc(motionFunc);
										 
	glutReshapeFunc(reshapeFunc);  // Set the reshape callback
	glutIdleFunc(idleFunc);
	
	// drawing callback
	glutDisplayFunc(display);



	// Go into forever loop and process events
	glutMainLoop();

	// never reached
	return 0;
}
