#pragma once

#ifndef POLYHEDRA_H_
#define POLYHEDRA_H_

#include <cstdlib> //Remove after
#include <glm/glm.hpp>

using namespace glm;

struct Vertex
{
	vec3 vertex;

};


class SimplePolyhedra
{

protected:
	//Vertices
	vec3 *_vertices;

	//Edge List


	//Faces
	//Direction Vectors


public:
	SimplePolyhedra()
	{

	}


	SimplePolyhedra()
	{
		delete[] _vertices;
	}
};






#endif