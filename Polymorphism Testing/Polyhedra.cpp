#include "Polyhedra.h"
#include <iostream>

#include "glm/glm.hpp"

Cube::Cube()
{

	
	_vertices = new vec3[_numverts]{ vec3(-1.0f,-1.0f,-1.0f),
									 vec3(-1.0f,-1.0f,1.0f),
									 vec3(1.0f,-1.0f,1.0f),
									 vec3(1.0f,-1.0f,-1.0f),
									 vec3(-1.0f,1.0f,-1.0f),
									 vec3(-1.0f,1.0f,1.0f),
									 vec3(1.0f,1.0f,1.0f),
									 vec3(1.0f,1.0f,-1.0f)
									};


	
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


	
	_faces = new int[_numverts]{
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



RhombicDodecahedron::RhombicDodecahedron()
{
	

	_vertices =  new vec3[_numverts]{
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

}