#pragma once

#ifndef POLYHEDRA_H_
#define POLYHEDRA_H_

#include <cstdlib> //Remove after
#include <glm/glm.hpp>
#include <iostream>
using namespace glm;


class SimplePolyhedra
{

protected:
	//Vertices
	const int _numverts;
	vec3 *_vertices;

	//Edge List
	const int _numedges;
	int *_edges;

	//Faces
	const int _numfaces;
	int *_faces;

	//Direction Vectors
	const int _numdirectionvectors;
	vec3 *_directionvectors;

public:
	SimplePolyhedra() : _numverts(0), _numedges(0), _numfaces(0), _numdirectionvectors(0){


	}

	SimplePolyhedra(int nverts, int nedges, int nfaces, int ndirectionv) : _numverts(nverts),_numedges(nedges), _numfaces(nfaces), _numdirectionvectors(ndirectionv){
			
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
	Cube() : SimplePolyhedra(8,12,6,3){

	}

	~Cube()
	{

	}


};


class RhombicDodecahedron : public SimplePolyhedra
{
protected:

public:
	RhombicDodecahedron() : SimplePolyhedra(14, 26, 12, 6) {

	}

	~RhombicDodecahedron()
	{

	}


};






#endif