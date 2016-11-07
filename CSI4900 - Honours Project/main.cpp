#define NOMINMAX
#include <cstdlib>
#include <cmath>
#include <bitset>
#include <cassert>
#include <iostream>
#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <GL/glew.h>
#include <GL/glut.h>
#include <stack>
#include <fstream>
#include <functional>
#include <algorithm>
#include <set>
// glm types
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
// matrix manipulation
#include <glm/gtc/matrix_transform.hpp>
// value_ptr
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/hash.hpp>
#include "shader.h"
#include "Polyhedra.h"
/** Global variables */

using namespace std;
using namespace glm;

typedef pair<int, int> edge;


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

	WindowSize() : d_near(1.0f), d_far(10.0f),d_perspective(false),

		d_widthPixel(512), d_width(30.0f),
		d_heightPixel(512), d_height(30.0f)
	{}
};
WindowSize g_winSize;

/**
* Camera properties
*/
struct Camera {
	GLboolean left_drag;
	GLint last_x, last_y;
	GLfloat vertical_rotation, horizontal_rotation;

	glm::vec3 camera_target;
	glm::vec3 camera_eye, camera_up;
	glm::vec3 target_to_camera;

	Camera() : left_drag(false), 
		
		last_x(0), last_y(0),
		vertical_rotation(0.0f), horizontal_rotation(0.0f),
		camera_target(0.0f,0.0f,0.0f), 
		camera_eye(0.0f,0.0f, -12.0f),
		camera_up(0.0f, 1.0f, 0.0f) {}


	void updateCameraVectors() {
		// Create the new camera up and right vectors
		glm::vec3 camera_right = glm::normalize(glm::cross(camera_eye, camera_up));
		camera_up = glm::normalize(glm::cross(camera_right, camera_eye));

		// Rotate the eye accordingly
		camera_eye = glm::rotate(camera_eye, horizontal_rotation, camera_up);
		camera_eye = glm::rotate(camera_eye, vertical_rotation, camera_right);
	}
};
Camera g_camera;

GLuint g_program;
GLuint* g_bufferObjects;


unordered_map <int, vector<vector<pair<edge, edge>>>> hashTable; //Stores hashes to speed up computations later

GLuint total_triangles;

/********************************/
int currentHash = 0;
int table[256];

void drawShape(SimplePolyhedra p, vec3 center) {

	glBegin(GL_LINES);

	for (int i = 0; i < p._numverts; ++i) {


		int edge = p._edges[i];

		for (int j = 0; j < p._numverts; ++j) {
			if (((1 << j) & edge) != 0) {

				glVertex3fv(glm::value_ptr(p._vertices[i] + center));
				glVertex3fv(glm::value_ptr(p._vertices[j] + center));


			}
		}
	}


	glEnd();


}


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

	//return (p1+p2)/2.0f; //midpoint formula
}

void marchPolygon(SimplePolyhedra p, glm::vec3 center) {
	
	GLfloat isolevel = 100.0f;
	int vertHash = 0; //This is an  integer to track the vertices contained in the isosurface
	
	for (int i = 0; i < p._numverts; i++) {
		//If vertex is within the isosurface
		if (glm::dot(p._vertices[i] + center, p._vertices[i] + center) <= isolevel) {
			vertHash = vertHash | 1 << i;
		}
	}
	
	int comp = (1<<(p._numverts)) - 1;
	
	//table[vertHash]++;
	//Do the following when the cube is neither fully in or out of the sphere
	if (vertHash != 0 && vertHash != comp) {


		if (hashTable.count(vertHash) < 1) {
			//The entry hasn't been made
			vector<vector<pair<edge, edge>>> hashEntry;

			//Find each plane of intersection on cube this is each vector<pair<int,in>> entry in the hashTable
			vector<int> planes = vector<int>();


			//Probably move this into higher scope
			function<int(SimplePolyhedra p, int vert, int hash)> floodFill;
			floodFill = [&floodFill](SimplePolyhedra p, int vert, int hash) {
				int res = 0;
				if ((1 << vert) & hash) { //vert in hash
					res = (1 << vert);
					for (int e = 0; e < p._numverts; ++e) { //for each vert e in p
						if ((1 << e) & p._edges[vert]) { //if the vert shares an edge with i
							if ((1 << e) & hash) { //if the i-th vert is in the hash
								res |= floodFill(p, e, (hash & ~(1 << vert)));
							}
						}
					}
				}
				return res;
			};

			for (int i = 0; i < p._numverts; ++i) {
				bool iAccountedFor = false;
				for (int pl : planes) {
					if ((1 << i) & pl) {
						iAccountedFor = true;
						break;
					}
				}

				if (!iAccountedFor) {
					int pHash = floodFill(p, i, vertHash);
					if (pHash != 0) {
						planes.push_back(pHash);
					}
				}
			}


			//For each distinct cut of the polyhedra

			int hashEntryIndex = 0;

			for (std::vector<int>::iterator it = planes.begin(); it != planes.end(); ++it) {

				hashEntry.push_back(vector<pair<edge, edge>>()); //New wrinkle-plane to be added



				//TO DO FIX AND MAKE READABLE
				vector<edge> crossedges = vector<edge>();

				for (int i = 0; i < p._numverts; i++) { //Iterate for each vertice contained under the plane
					if ((*it & 1 << i) != 0) {
						int edgeHash = ~*it & p._edges[i];
						for (int e = 0; e < p._numverts; e++) {
							//Check the edges with one vertice in and one vertice out in the cube
							if (edgeHash >> e & 1) {
								crossedges.push_back((edge(i, e)));
							}

						}
					}

				}

				//for each edge find a pair of edges sharing a face (only need one direction)
				vector<pair<edge, edge>> triangle = vector<pair<edge, edge>>();

				for (int i = 0; i < crossedges.size() - 1; i++) {
					for (int j = i + 1; j < crossedges.size(); j++) {

						//For each face check if the edges share the face
						for (int f = 0; f < p._numfaces; f++) {

							int tf = (1 << crossedges[i].first | 1 << crossedges[i].second) | (1 << crossedges[j].first | 1 << crossedges[j].second);
							if ((p._faces[f] | tf) == p._faces[f]) {

								//They are on the same face
								triangle.push_back(make_pair(crossedges[i], crossedges[j]));

								hashEntry[hashEntryIndex].push_back(std::make_pair(crossedges[i], crossedges[j]));
								break;
							}

						}

					}

				}
			}


			hashTable[vertHash] = hashEntry;

		}
		//We now definitely have the key, and just create the elements then


		for (vector<pair<edge, edge>> cut : hashTable[vertHash]) {

			glm::vec3 centroid = glm::vec3(0.0f);

			//cut is a pair of edges that mixed with a centroid forms a triangle
			for (pair<edge, edge> crossedges : cut) {

				glm::vec3 a, b;

				a = p._vertices[crossedges.first.first] + center;
				b = p._vertices[crossedges.first.second] + center;
				centroid += interpolate(isolevel, a, b, glm::dot(a, a), glm::dot(b, b));


				a = p._vertices[crossedges.second.first] + center;
				b = p._vertices[crossedges.second.second] + center;
				centroid += interpolate(isolevel, a, b, glm::dot(a, a), glm::dot(b, b));
			}

			centroid /= (GLfloat)(2 * cut.size());

			glBegin(GL_TRIANGLES); //USE GL_TRIANGLES or GL_LINE_LOOP

			for (pair<edge, edge> triangle : cut) {

				glVertex3fv(glm::value_ptr(centroid));

				glm::vec3 a1, a2, b1, b2;

				a1 = p._vertices[triangle.first.first] + center;
				b1 = p._vertices[triangle.first.second] + center;
				glVertex3fv(glm::value_ptr(interpolate(isolevel, a1, b1, glm::dot(a1, a1), glm::dot(b1, b1))));


				a2 = p._vertices[triangle.second.first] + center;
				b2 = p._vertices[triangle.second.second] + center;
				glVertex3fv(glm::value_ptr(interpolate(isolevel, a2, b2, glm::dot(a2, a2), glm::dot(b2, b2))));

			}

			++total_triangles;
			glEnd();

		}
	}
			
}
std::ofstream outfile; //Debugging stream
unordered_set<vec3> processedPositions;


void lincomb(SimplePolyhedra p, vec3 origin, int depth, GLint *linc) {

	if (depth == 0) {

		vec3 center = origin;

		for (int d = 0; d < p._numdirectionvectors; ++d) {
			center += (GLfloat) (linc[d]) * p._directionvectors[d];
		}

		
		//outfile << "p " << center.x << " " << center.y << " " << center.z << endl;

		//**TODO -- Poor solution needs to be properly analysed
		if (processedPositions.find(center) != processedPositions.end()) {
			//We have not already encountered this position
			marchPolygon(p, center);
			processedPositions.insert(center);
			//drawShape(p, center);
		}
		
		
		
	}
	else {

		//TODO Test for bounding box
		int size = 6;
		for (int i = -size; i <= size; ++i) {
			linc[depth - 1] = i; //determines base case to be 0 otherwise we have 0-1
			lincomb(p, origin, depth-1, linc);
		}

	}
}


void recursiveFaceOrientedSpherePacking(SimplePolyhedra p, vec3 pos) {

	//check within bounding box

	if (abs(pos.x) > 10.0f || abs(pos.y) > 10.0f || abs(pos.z) > 10.0f) {
		//We are not going to render this position
		return;
	}
	else if (processedPositions.find(pos) != processedPositions.end()){
		//We have processed this position. No need to render.
		return;
	}
	else {
		marchPolygon(p, pos);
		processedPositions.insert(pos);
		//drawShape(p, pos);

		for (int i = 0; i < p._numdirectionvectors; ++i) {
			recursiveFaceOrientedSpherePacking(p, pos + p._directionvectors[i]);
			recursiveFaceOrientedSpherePacking(p, pos - p._directionvectors[i]);
		}
	}




}

void recursiveMarch(SimplePolyhedra p) {
	//SimplePolyhedra p should be a Cube or RhombicDodecahedron atm.


	//outfile.open(p.label + " pos.txt");
	processedPositions = unordered_set<vec3>();
	//lincomb(p, vec3(0.0f, 0.0f, 0.0f), p._numdirectionvectors, new GLint[p._numdirectionvectors]);
	
	recursiveFaceOrientedSpherePacking(p, vec3(0.0f, 0.0f, 0.0f));
	//outfile.close();

}

void marchingCubes() {

	//Direction vectors of a cube
	//This is an array s.t. you can tile 3d with a linear combination of these centers
	glm::vec3 d[] = { glm::vec3(2.0f,0.0f,0.0f) ,glm::vec3(0.0f,2.0f,0.0f) ,glm::vec3(0.0f,0.0f,2.0f) };
	
	

	int size = 6;
	for (int i = -size; i < size; i++)
		for (int j = -size; j < size; j++)
			for (int k = -size; k < size; k++) {
				
				glm::vec3 center = (GLfloat)(i)*d[0] + (GLfloat)(j)* d[1] + (GLfloat)(k)* d[2];
				
				/*
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, center);
				glUniformMatrix4fv(g_tfm.locM, 1, GL_FALSE, glm::value_ptr(model));
				glutWireCube(2.0f);
				*/
				//March Cubes
				//drawShape(Cube(), center);
				marchPolygon(Cube(), center);

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
	if (g_camera.left_drag) g_camera.updateCameraVectors(); //Don't update camera unless the camera is being moved
	glm::mat4 View = glm::lookAt(g_camera.camera_eye, g_camera.camera_target, g_camera.camera_up);
	glm::mat4 Model = glm::mat4(1.0f);

	/*
	string strTri = to_string(total_triangles);
	glRasterPos2f(10, 10);
	for (char c : strTri) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
	}*/


	// Update uniform for this drawing
	
	glUniformMatrix4fv(g_tfm.locV, 1, GL_FALSE, glm::value_ptr(View));
	glUniformMatrix4fv(g_tfm.locM, 1, GL_FALSE, glm::value_ptr(Model));


	/**TODO
		Decide when to choose isosurface to be rendered
	*/

	total_triangles = 0;
	SimplePolyhedra poly = RhombicDodecahedron();
	
	recursiveMarch(poly);

	
	
	

	cout << "Finish display" << endl;
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
		g_camera = Camera();
		break;
	case 'i':
	case 'I':
		currentHash = (currentHash + 1);
		cout << "CurrentHash: " << currentHash << " (" << bitset<14>(currentHash) << ")" << endl;
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
		g_camera.left_drag = true;

		g_camera.last_x = x;
		g_camera.last_y = y;

		glutPostRedisplay();
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		g_camera.left_drag = false;

		glutPostRedisplay();
	}


}

// mouse button callback
void motionFunc(int x, int y) {
	if (g_camera.left_drag) {
		g_camera.horizontal_rotation = (GLfloat)(g_camera.last_x - x) * 0.01f;
		g_camera.vertical_rotation = (GLfloat)(y - g_camera.last_y) * 0.01f;

		g_camera.last_x = x;
		g_camera.last_y = y;

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
