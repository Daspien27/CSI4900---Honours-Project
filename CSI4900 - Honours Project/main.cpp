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
#include "light.h"
#include "Isosurface.h"
#include "main.h"
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
	GLint locCol;
	GLint locTexture;
	Attributes() : locPos(-1), locNorm(-1), locCol(-1), locTexture(-1) {}
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

	WindowSize() : d_near(1.0f), d_far(400.0f),d_perspective(false),

		d_widthPixel(512), d_width(40.0f),
		d_heightPixel(512), d_height(40.0f)
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
		camera_eye(0.0f,0.0f, -50.0f),
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

/*Storage for a Model's Buffers*/
struct ModelBuffers {
	GLuint vao;

	GLuint vbo;
	vector<glm::vec3> vertices;
	
	//TODO: look integers normalized integers for normals
	GLuint nbo;
	vector<glm::vec3> normals;

	GLuint cbo;
	vector<glm::vec3> colours;
};
ModelBuffers g_buffers;

GLuint g_program;


GLfloat scalingFactor =1.0f;
GLint threshold = 128;

glm::vec3 origin = vec3(0.0f, 0.0f, 0.0f); //Offsets the center of the marching polyhedra
SimplePolyhedra g_poly = RhombicDodecahedron(scalingFactor);

Isosurface& g_mySurface = isoMRI(threshold);
GLint g_renderStyle = GL_TRIANGLES; //USE GL_TRIANGLES or GL_LINE_LOOP



unordered_map <int, vector<vector<pair<edge, edge>>>> hashTable; //Stores hashes to speed up computations later

GLuint total_triangles;
GLuint total_polyhedra;
GLuint total_crossections;


GLfloat approximateSA;
GLfloat approximateVol;






// Lighting
GLint g_cLight = 0;
LightArray g_lightArray;
GLfloat g_lightAngle = 0.0f;



void drawShape(SimplePolyhedra p, vec3 center) {

	glBegin(GL_LINES);

	for (int i = 0; i < p._numverts; ++i) {


		int edge = p.getEdge(i);

		for (int j = 0; j < p._numverts; ++j) {
			if (((1 << j) & edge) != 0) {

				glVertex3fv(glm::value_ptr(p.getVertice(i) + center));
				glVertex3fv(glm::value_ptr(p.getVertice(j) + center));


			}
		}
	}


	glEnd();


}


glm::vec3 interpolate(glm::vec3 p1, glm::vec3 p2, GLfloat valp1, GLfloat valp2) {
	//Formula from http://paulbourke.net/geometry/polygonise/
	glm::vec3 p;

	if (glm::abs(valp1) < 0.00001)return(p1);
	if (glm::abs(valp2) < 0.00001)return(p2);
	if (glm::abs(valp1 - valp2) < 0.00001)  return(p1);
	
	GLfloat mu = (-valp1) / (valp2 - valp1);
	
	p = p1 + mu * (p2 - p1);

	return(p);

	//return (p1+p2)/2.0f; //midpoint formula
}


GLfloat areaOfTriangle(vec3 A, vec3 B, vec3 C) {
	//Implemented via Heron's formula

	GLfloat lenAB = sqrt(dot(B - A, B - A));
	GLfloat lenBC = sqrt(dot(C - B, C - B));
	GLfloat lenCA = sqrt(dot(A - C, A - C));

	GLfloat s = (lenAB + lenBC + lenCA)/2;

	return sqrt(s*(s-lenAB)*(s-lenBC)*(s-lenCA));
}

GLfloat volTrianglebasedPyramid(vec3 A, vec3 B, vec3 C) {

	return abs(dot(A, cross(B,C)))/6.0f;
}

void marchPolygon(SimplePolyhedra p, glm::vec3 center, Isosurface& isosurface) {
	
	int vertHash = 0; //This is an  integer to track the vertices contained in the isosurface
	for (int i = 0; i < p._numverts; i++) {
		//If vertex is within the isosurface
		if (isosurface(p.getVertice(i) + center) <= 0.0f) {
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
						if ((1 << e) & p.getEdge(vert)) { //if the vert shares an edge with i
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
						int edgeHash = ~*it & p.getEdge(i);
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
							if ((p.getFace(f) | tf) == p.getFace(f)) {

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

				a = p.getVertice(crossedges.first.first) + center;
				b = p.getVertice(crossedges.first.second) + center;
				centroid += interpolate(a, b, isosurface(a), isosurface(b));


				a = p.getVertice(crossedges.second.first) + center;
				b = p.getVertice(crossedges.second.second) + center;
				centroid += interpolate(a, b, isosurface(a), isosurface(b));
			}

			centroid /= (GLfloat)(2 * cut.size());


			for (pair<edge, edge> triangle : cut) {

				glm::vec3 a1, a2, b1, b2, v1, v2;

				a1 = p.getVertice(triangle.first.first) + center;
				b1 = p.getVertice(triangle.first.second) + center;
				v1 = interpolate(a1, b1, isosurface(a1), isosurface(b1));

				a2 = p.getVertice(triangle.second.first) + center;
				b2 = p.getVertice(triangle.second.second) + center;
				v2 = interpolate(a2, b2, isosurface(a2), isosurface(b2));
			
				g_buffers.vertices.push_back(centroid);
				g_buffers.vertices.push_back(v1);
				g_buffers.vertices.push_back(v2);
				
				//Area checks
				GLfloat area = areaOfTriangle(centroid, v1, v2);
				GLfloat volOfPyramid = volTrianglebasedPyramid(centroid, v1, v2);
				
				approximateSA += area;
				approximateVol += volOfPyramid;

				vec3 d1 = v1 - centroid;
				vec3 d2 = v2 - centroid;

				vec3 n = 1.0f * cross(d2, d1);
				
				g_buffers.normals.push_back(n);
				g_buffers.normals.push_back(n);
				g_buffers.normals.push_back(n);
				
				vec3 middle = (centroid + v1 + v2)/3.0f;

				vec3 colour = vec3(1.0f, 1.0f, 0.0f);//(isosurface(middle)+128)/256.0f*glm::vec3(1.0f, 0.8f, 0.8f);
				g_buffers.colours.push_back(colour);
				g_buffers.colours.push_back(colour);
				g_buffers.colours.push_back(colour);
			
				
				++total_triangles;
			}
			++total_crossections;
		}

		

	}
			
}


bool notInRenderBounds(vec3 pos, Isosurface& surface) {
	//Will do for now
	return (abs(pos.x) > 10.0f || abs(pos.y) > 10.0f || abs(pos.z) > 10.0f);
}

void recursiveMarch(SimplePolyhedra p, Isosurface& surface) {
	
	g_buffers.vertices.clear();
	g_buffers.normals.clear();
	g_buffers.colours.clear();

	stack<vec3> polyhedraStack;
	polyhedraStack.push(origin); //Push the center of the march into the stack
	unordered_set<vec3> processedPositions; //closed positions
	
	while (!polyhedraStack.empty()) {
		//Pop first position
		vec3 pos = polyhedraStack.top();
		polyhedraStack.pop();

		if (notInRenderBounds(pos, surface)) {
			//Way outside of isosurface and we don't need it

		}
		else if (processedPositions.find(pos) != processedPositions.end()) {
			//We have seen the position before, no need to reprocess
		}
		else {
			//New point!
			
			processedPositions.insert(pos);
			marchPolygon(p, pos, surface);
			
			//drawShape(p, pos);
			//cout << "--B--" << endl;
			for (int i = 0; i < p._numdirectionvectors; ++i) {
				
				polyhedraStack.push(pos + p.getDirectionVector(i));
			//	cout << (pos + p.getDirectionVector(i)).x << " " << (pos + p.getDirectionVector(i)).y << " " << (pos + p.getDirectionVector(i)).z << endl;
				polyhedraStack.push(pos - p.getDirectionVector(i));
			//	cout << (pos - p.getDirectionVector(i)).x << " " << (pos - p.getDirectionVector(i)).y << " " << (pos - p.getDirectionVector(i)).z << endl;
			}
			//cout << "--E--" << endl;
			++total_polyhedra;

		}

		

	}

	//After we have recieved our vertices we will process them in the buffers

	//Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, g_buffers.vbo);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * static_cast<GLint>(g_buffers.vertices.size()),
		&g_buffers.vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(g_attrib.locPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_attrib.locPos);
	
	//Normal Buffer
	glBindBuffer(GL_ARRAY_BUFFER, g_buffers.nbo);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * static_cast<GLint>(g_buffers.normals.size()),
		&g_buffers.normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(g_attrib.locNorm, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_attrib.locNorm);
	
	//Colour Buffer
	glBindBuffer(GL_ARRAY_BUFFER, g_buffers.cbo);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * static_cast<GLint>(g_buffers.colours.size()),
		&g_buffers.colours[0], GL_STATIC_DRAW);
	glVertexAttribPointer(g_attrib.locCol, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_attrib.locCol);
	


	//Draw the buffers
	glDrawArrays(g_renderStyle, 0, g_buffers.vertices.size());
}

/**
* Display routine 
*/
void display(void) {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear the window
	//Direct camera using the coordinate's basis
	if (g_camera.left_drag) g_camera.updateCameraVectors(); //Don't update camera unless the camera is being moved
	glm::mat4 View = glm::lookAt(g_camera.camera_eye, g_camera.camera_target, g_camera.camera_up);
	glm::mat4 Model = glm::mat4(1.0f);
	// Update uniform for this frame	
	glUniformMatrix4fv(g_tfm.locV, 1, GL_FALSE, glm::value_ptr(View));
	glUniformMatrix4fv(g_tfm.locM, 1, GL_FALSE, glm::value_ptr(Model));

	//Taken from A3 from CSI4130 WINTER2016
	/*LightSource light = g_lightArray.get(g_cLight);
	std::ostringstream os;
	os << "lightPosition[" << g_cLight << "]";
	std::string varName = os.str();
	GLuint locLightPos = glGetUniformLocation(g_program, varName.c_str());
	glm::vec4 lightPos =
		glm::vec4(cos(g_lightAngle)*g_winSize.d_width,
			sin(g_lightAngle)*g_winSize.d_width,
			20.0f, // * static_cast<GLfloat>( !light.d_pointLight ), 
			static_cast<GLfloat>(light.d_pointLight));
	glProgramUniform4fv(g_program, locLightPos, 1, glm::value_ptr(lightPos));
	*/

	total_triangles = 0;
	total_crossections = 0;
	total_polyhedra = 0;
	
	approximateSA = 0.0f;
	approximateVol = 0.0f;
	
	
	recursiveMarch(g_poly, g_mySurface);
	//glutSolidSphere(6.0f,20,20);

	
	
	cout << "Isosurface: " << endl;
	cout << "\t" << g_mySurface.label() << endl;
	cout << "\t Exact Surface Area: " << g_mySurface.surfacearea() << endl;
	cout << "\t Exact Volume: " << g_mySurface.volume() << endl;
	cout << "Counts: " << endl;
	cout << "\t Number of polyhedra: " << total_polyhedra << endl;
	cout << "\t Number of crosssections: " << total_crossections << endl;
	cout << "\t Number of triangles: " << total_triangles << endl;
	cout << "Errors: " << endl;
	cout << "\t Approximate Surface Area: " << approximateSA << endl;
	cout << "\t Approximate Volume: " << approximateVol << endl;
	cout << "\t Standard SA Error: " << abs(g_mySurface.surfacearea() - approximateSA) << endl;
	cout << "\t Standard Vol Error: " << abs(g_mySurface.volume() - approximateVol) << endl;

	// swap buffers
	glutSwapBuffers();
}


/**
* Idle routine
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
	case 'p':
	case 'P':
		threshold++;
		g_mySurface = isoMRI(threshold);
		break;
	case 'O':
	case 'o':
		threshold--;
		g_mySurface = isoMRI(threshold);
		break;
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

/**
* Mouse button routine
*/
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

/**
* Mouse movement routine
*/
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
* Reshape function - main window
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
* Initialization
*/
void init(void) {
	// darkgray background
	glClearColor(0.4f, 0.4f, 0.4f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	
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


	// Load shaders
	vector<GLuint> sHandles;
	GLuint handle;
	Shader shader;
	if (!shader.load("march.vs", GL_VERTEX_SHADER)) {
		shader.installShader(handle, GL_VERTEX_SHADER);
		Shader::compile(handle);
		sHandles.push_back(handle);
	}
	if (!shader.load("march.fs", GL_FRAGMENT_SHADER)) {
		shader.installShader(handle, GL_FRAGMENT_SHADER);
		Shader::compile(handle);
		sHandles.push_back(handle);
	}
	cerr << "No of handles: " << sHandles.size() << endl;
	Shader::installProgram(sHandles, g_program);
	

	// Activate program in order to be able to set uniforms 
	glUseProgram(g_program);
	// vertex attributes
	g_attrib.locPos = glGetAttribLocation(g_program, "position");
	g_attrib.locNorm = glGetAttribLocation(g_program, "normal");
	g_attrib.locCol = glGetAttribLocation(g_program, "colour");
	// find the locations of our uniforms and store them in a global structure for later access
	g_tfm.locM = glGetUniformLocation(g_program, "ModelMatrix");
	g_tfm.locV = glGetUniformLocation(g_program, "ViewMatrix");
	g_tfm.locP = glGetUniformLocation(g_program, "ProjectionMatrix");
	
	// Generate a VAO
	glGenVertexArrays(1, &g_buffers.vao);
	glBindVertexArray(g_buffers.vao);

	//Vertex Buffer
	glGenBuffers(1, &g_buffers.vbo);
	glGenBuffers(1, &g_buffers.nbo);
	glGenBuffers(1, &g_buffers.cbo);
	
	//Just one light source
	g_lightArray.append(LightSource());
	g_lightArray.setLights(g_program);
	g_lightArray.setPositions(g_program);
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
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

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
	glutKeyboardFunc(keyboardFunc); // Handles standard keys
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
