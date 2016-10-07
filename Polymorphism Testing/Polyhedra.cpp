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
	
	_numverts = 14;
	_vertices = new vec3[_numverts]{ 
		vec3(0.0f,-2.0f,0.0f),
		
		vec3(-1.0f,-1.0f,-1.0f),
		vec3(-1.0f,-1.0f,1.0f),
		vec3(1.0f,-1.0f,1.0f),
		vec3(1.0f,-1.0f,-1.0f),

		vec3(-2.0f,0.0f,0.0f),
		vec3(0.0f,0.0f,2.0f),
		vec3(2.0f,0.0f,0.0f),
		vec3(0.0f,0.0f,-2.0f),

		vec3(-1.0f,1.0f,-1.0f),
		vec3(-1.0f,1.0f,1.0f),
		vec3(1.0f,1.0f,1.0f),
		vec3(1.0f,1.0f,-1.0f),
		
		vec3(0.0f,2.0f,0.0f)
		
	};


	_numedges = 24;
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
			0b01111000000000,
	};


	_numfaces = 12;
	_faces = new int[_numverts] {
		0b00000001001101,
			0b00000010011001,
			0b00000100000111,
			0b00000000101101,
			0b00100011001000,
			0b01000110010000,
			0b00001100100010,
			0b00010001100100,
			0b10110010000000,
			0b11100001000000,
			0b11001100000000,
			0b10011000100000

	};






}