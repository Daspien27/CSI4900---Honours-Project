#ifndef POLYHEDRA_H_
#define POLYHEDRA_H_


#include <GL/glew.h>
#include <GL/glut.h>
#include <cstdlib> //Remove after
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <unordered_set>
#include <assert.h>
using namespace glm;



class SimplePolyhedra
{

public:
	const std::string label;
	const int _numverts;
	const int _numedges;
	const int _numfaces;
	const int _numdirectionvectors;
	const GLfloat uniformScaling;
protected:
	//Vertices
	vec3 *_vertices;
	//Edge List
	int *_edges;
	//vector< list<int> > _edgeAdjancencyList;
	//Faces
	int *_faces;
	//vector< unordered_set<int> > _faceSets;
	//Direction Vectors
	vec3 *_directionvectors;
public:
	SimplePolyhedra() : _numverts(0), _numedges(0), _numfaces(0), _numdirectionvectors(0), label("SimplePolyhedra"), uniformScaling(1.0f){


	}
	SimplePolyhedra(int nverts, int nedges, int nfaces, int ndirectionv, std::string label, GLfloat scaling) : _numverts(nverts),_numedges(nedges), _numfaces(nfaces), _numdirectionvectors(ndirectionv), label(label), uniformScaling(scaling){
			
	}
	~SimplePolyhedra()
	{
		//delete[] _vertices;
	}

	inline vec3 *getVertices() {
		return _vertices;
	}

	
	inline vec3 getVertice(int i) {
		assert(i >= 0 && i <= _numvertices);
		return uniformScaling * _vertices[i];
	}

	inline int getEdge(int i) {
		assert(i >= 0 && i <= _numverts);
		return _edges[i];
	}

	inline int getFace(int i) {
		assert(i >= 0 && i <= _numverts);
		return _faces[i];
	}

	inline vec3 getDirectionVector(int i) {
		assert(i >= 0 && i <= _numdirectionvectors);
		return uniformScaling * _directionvectors[i];
	}
	
};

class Cube : public SimplePolyhedra
{
protected:
	
public:
	Cube(GLfloat scaling) : SimplePolyhedra(8, 12, 6, 3, "Cube",scaling) {

		_vertices = new vec3[_numverts]{ 
			vec3(-1.0f,-1.0f,-1.0f),
			vec3(-1.0f,-1.0f,1.0f),
			vec3(1.0f,-1.0f,1.0f),
			vec3(1.0f,-1.0f,-1.0f),
			vec3(-1.0f,1.0f,-1.0f),
			vec3(-1.0f,1.0f,1.0f),
			vec3(1.0f,1.0f,1.0f),
			vec3(1.0f,1.0f,-1.0f)
		};



		_edges = new int[_numverts] {
				0b00011010,
				0b00100101,
				0b01001010,
				0b10000101,
				0b10100001,
				0b01010010,
				0b10100100,
				0b01011000,
		};

		/*
		_edgeAdjancencyList.push_back(list<int>{1, 3, 4});
		_edgeAdjancencyList.push_back(list<int>{0, 2, 5});
		_edgeAdjancencyList.push_back(list<int>{1, 3, 6});
		_edgeAdjancencyList.push_back(list<int>{0, 2, 7});
		_edgeAdjancencyList.push_back(list<int>{0, 5, 7});
		_edgeAdjancencyList.push_back(list<int>{1, 4, 6});
		_edgeAdjancencyList.push_back(list<int>{2, 5, 7});
		_edgeAdjancencyList.push_back(list<int>{3, 4, 6});
		*/


		_faces = new int[_numverts] {
				0b00001111,
				0b00110011,
				0b01100110,
				0b11001100,
				0b10011001,
				0b11110000

		};
		/*
		_faceSets.push_back(unordered_set<int>{ 0, 1, 2, 3 });
		_faceSets.push_back(unordered_set<int>{ 0, 1, 4, 5 });
		_faceSets.push_back(unordered_set<int>{ 1, 2, 5, 6 });
		_faceSets.push_back(unordered_set<int>{ 2, 3, 6, 7 });
		_faceSets.push_back(unordered_set<int>{ 0, 3, 4, 7 });
		_faceSets.push_back(unordered_set<int>{ 4, 5, 6, 7 });
		*/

		_directionvectors = new vec3[_numdirectionvectors]{
			vec3(2.0f,0.0f,0.0f),
			vec3(0.0f,2.0f,0.0f),
			vec3(0.0f,0.0f,2.0f)
		};


	}

	//Cube() : Cube(1.0f) {}

	~Cube()
	{

	}


};

class HexagonalPrism : public SimplePolyhedra
{
protected:

public:
	HexagonalPrism(GLfloat scaling) : SimplePolyhedra(8, 12, 6, 3, "HexagonalPrism", scaling) {

		_vertices = new vec3[_numverts]{
			vec3(-1.0f,-1.0f,-1.0f),
			vec3(-1.0f,-1.0f,1.0f),
			vec3(1.0f,-1.0f,1.0f),
			vec3(1.0f,-1.0f,-1.0f),
			vec3(-1.0f,1.0f,-1.0f),
			vec3(-1.0f,1.0f,1.0f),
			vec3(-1.0f,-1.0f,-1.0f),
			vec3(-1.0f,-1.0f,1.0f),
			vec3(1.0f,-1.0f,1.0f),
			vec3(1.0f,-1.0f,-1.0f),
			vec3(-1.0f,1.0f,-1.0f),
			vec3(-1.0f,1.0f,1.0f),
		};



		_edges = new int[_numverts] {
			0b00011010,
				0b00100101,
				0b01001010,
				0b10000101,
				0b10100001,
				0b01010010,
				0b10100100,
				0b01011000,
		};


		_faces = new int[_numverts] {
			0b00001111,
				0b00110011,
				0b01100110,
				0b11001100,
				0b10011001,
				0b11110000

		};
	

		_directionvectors = new vec3[_numdirectionvectors]{
			vec3(2.0f,0.0f,0.0f),
			vec3(0.0f,2.0f,0.0f),
			vec3(0.0f,0.0f,2.0f)
		};


	}

	

	~HexagonalPrism()
	{

	}


};
class RhombicDodecahedron : public SimplePolyhedra
{
protected:

public:
	RhombicDodecahedron(GLfloat scaling) : SimplePolyhedra(14, 24, 12, 6, "RhombicDodecahedra", scaling) {

		_vertices = new vec3[_numverts]{
			vec3(0.0f, 2.0f, 0.0f),

			vec3(1.0f, 1.0f, 1.0f),
			vec3(1.0f, 1.0f,-1.0f),
			vec3(-1.0f, 1.0f,-1.0f),
			vec3(-1.0f, 1.0f, 1.0f),

			vec3(2.0f, 0.0f, 0.0f),
			vec3(0.0f, 0.0f,-2.0f),
			vec3(-2.0f, 0.0f, 0.0f),
			vec3(0.0f, 0.0f, 2.0f),

			vec3(1.0f,-1.0f, 1.0f),
			vec3(1.0f,-1.0f,-1.0f),
			vec3(-1.0f,-1.0f,-1.0f),
			vec3(-1.0f,-1.0f, 1.0f),

			vec3(0.0f,-2.0f, 0.0f)

		};

		_edges = new int[_numverts] {
			0b00000000011110,

				0b00000100100001,
				0b00000001100001,
				0b00000011000001,
				0b00000110000001,

				0b00011000000110,
				0b00110000001100,
				0b01100000011000,
				0b01001000010010,

				0b10000100100000,
				0b10000001100000,
				0b10000011000000,
				0b10000110000000,

				0b01111000000000
		};


		_faces = new int[_numfaces] {
			0b00000000100111,
				0b00000001001101,
				0b00000010011001,
				0b00000100010011,

				0b00001100100010,
				0b00010001100100,
				0b00100011001000,
				0b01000110010000,

				0b10011000100000,
				0b10110001000000,
				0b11100010000000,
				0b11001100000000
		};


		
		_directionvectors = new vec3[_numdirectionvectors]{
			cross((_vertices[0] - _vertices[1]), (_vertices[0] - _vertices[2])),
			cross((_vertices[0] - _vertices[2]), (_vertices[0] - _vertices[3])),
			cross((_vertices[0] - _vertices[3]), (_vertices[0] - _vertices[4])),
			cross((_vertices[0] - _vertices[4]), (_vertices[0] - _vertices[1])),
			cross((_vertices[1] - _vertices[5]), (_vertices[1] - _vertices[8])),
			cross((_vertices[2] - _vertices[5]), (_vertices[2] - _vertices[6]))
		};
		
		/*
		_directionvectors = new vec3[_numdirectionvectors]{
			2.0f*normalize(_vertices[0] - _vertices[1]),
			2.0f*normalize(_vertices[0] - _vertices[2]),
			2.0f*normalize(_vertices[0] - _vertices[3]),
			2.0f*normalize(_vertices[0] - _vertices[4])
		};
		*/
	}
	//RhombicDodecahedron(): RhombicDodecahedron(1.0f) { }

	~RhombicDodecahedron()
	{

	}


};






#endif