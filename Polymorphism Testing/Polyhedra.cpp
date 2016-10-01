#include "Polyhedra.h"
#include <iostream>

#include "glm/glm.hpp"

Cube::Cube()
{

	_numverts = 8;
	_vertices = new vec3[_numverts]{ vec3(-1.0f,-1.0f,-1.0f),
									 vec3(-1.0f,-1.0f,1.0f),
									 vec3(1.0f,-1.0f,1.0f),
									 vec3(1.0f,-1.0f,-1.0f),
									 vec3(-1.0f,1.0f,-1.0f),
									 vec3(-1.0f,1.0f,1.0f),
									 vec3(1.0f,1.0f,1.0f),
									 vec3(1.0f,1.0f,-1.0f)
									};


	_numedges = 12;
	_edges = new int[_numverts]{
		0b00011010,
		0b00100101,
		0b01001010,
		0b10000101,
		0b10100001,
		0b01010010,
		0b10100100,
		0b01011000,
	};


	_numfaces = 6;
	_faces = new int[_numverts]{
		0b00001111,
		0b00110011,
		0b01100110,
		0b11001100,
		0b10011001,
		0b11110000

	};

	_numdirectionvectors = 3;
	_directionvectors = new vec3[_numdirectionvectors]{ 
								vec3(2.0f,0.0f,0.0f),
								vec3(0.0f,2.0f,0.0f),
								vec3(0.0f,0.0f,2.0f)
	};

}



RhombicDodecahedron::RhombicDodecahedron()
{
	_vertices = new vec3[1]{ vec3(0,0,0) };
}
