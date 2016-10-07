#include <cstdlib>
#include <iostream>

#include "Polyhedra.h"

using namespace std;



void recursive_depth(int depth, SimplePolyhedra p, int *lincomb, int bound) {

	if (depth == 0) {

		//march

	}
	else {

		for (int i = -bound; i <= bound; ++i) {
			
			lincomb[depth - 1] = i;
			recursive_depth(depth - 1, p, lincomb, bound);

		}



	}


}


void testmarchingpolyhedra(SimplePolyhedra p) {

	p.print();


	recursive_depth(p._numdirectionvectors, p, new int[p._numdirectionvectors], 3);



}



int main (){

	cout << "HI" << endl;
	
	Cube myCube = Cube();
	RhombicDodecahedron myDodecahedron = RhombicDodecahedron();


	myCube.print();
	myDodecahedron.print();


	testmarchingpolyhedra(myCube);

	while (true);
	return 0;
}