#pragma once

#ifndef POLYHEDRA_H_
#define POLYHEDRA_H_

#include <cstdlib> //Remove after
#include <glm/glm.hpp>
#include <iostream>
using namespace glm;


class SimplePolyhedra
{

public:
	//Vertices
	int _numverts;
	vec3 *_vertices;

	//Edge List
	int _numedges;
	int *_edges;

	//Faces
	int _numfaces;
	int *_faces;

	//Direction Vectors
	int _numdirectionvectors;
	vec3 *_directionvectors;

public:
	SimplePolyhedra()
	{

	}


	~SimplePolyhedra()
	{
		//delete[] _vertices;
	}


	virtual void print() {

		std::cout << "SimplePolyhedra: " <<  _numverts << std::endl;

		for (int i = 0; i < _numverts; ++i) {

			std::cout << "\t " << _vertices[i].x << " " << _vertices[i].y << " " << _vertices[i].z << std::endl;


		}

	}
};

class Cube : public SimplePolyhedra
{
protected:

public:
	Cube();

	~Cube()
	{

	}


};


class RhombicDodecahedron : public SimplePolyhedra
{
protected:

public:
	RhombicDodecahedron();

	~RhombicDodecahedron()
	{

	}


};






#endif