#ifndef ISOSURFACE_H_
#define ISOSURFACE_H_

using namespace glm;

class Isosurface {
public:
	virtual char* label() { return "Isosurface";  }
	virtual GLfloat operator()(vec3 v) { return false; }
	virtual GLfloat surfacearea() { return -1.0f; }
	virtual GLfloat volume() { return -1.0f; }
};

class isoSphere : public Isosurface {
private:
	GLfloat radius_pow2;
public:
	isoSphere(GLfloat r) : radius_pow2(r*r) {

	}

	GLfloat operator()(vec3 point) {

		GLfloat pointpos = dot(point, point);

		return pointpos - radius_pow2;
	}

	char* label() { return "Sphere"; }

	GLfloat surfacearea() {
		//(atan(1.0f)*4.0f) is PI
		return 4.0f*radius_pow2*(atan(1.0f)*4.0f);
	}

	GLfloat volume() {
		//(atan(1.0f)*4.0f) is PI
		return 4.0f*(atan(1.0f)*4.0f)*pow(radius_pow2, 1.5f) / 3.0f;
	}

};
class isoCube : public Isosurface {
private:
	GLfloat length;
public:
	isoCube(GLfloat r) : length(r) {

	}

	GLfloat operator()(vec3 point) {
		GLfloat minX = abs(point.x);
		GLfloat minY = abs(point.y);
		GLfloat minZ = abs(point.z);

		if (minX >= minY && minX >= minZ) {

			return minX - length;
		}
		else if (minY >= minZ) {

			return minY - length;
		}
		else {
			return minZ - length;
		}
	}

	char* label() { return "Cube"; }
	GLfloat surfacearea() {
		//(atan(1.0f)*4.0f) is PI
		return 6*4*length*length;
	}

	GLfloat volume() {
		//(atan(1.0f)*4.0f) is PI
		return 8*length*length*length;
	}

};
class isoMRI : public Isosurface {
	//Based on volexample file, for now
#define NX 200
#define NY 160
#define NZ 160
private:
	vector<vector<vector<short int>>> data;
	short int isolevel;
public:
	isoMRI(int iso) {
		//Taken word for word from my volexample.c
		isolevel = iso;

		FILE *fptr;

		fprintf(stderr, "Reading data ...\n");
		if ((fptr = fopen("mri.raw", "rb")) == NULL) {
			fprintf(stderr, "File open failed\n");
			exit(-1);
		}

		int c;
		short int themax = 0, themin = 255;
		data.resize(NZ);
		for (int k = 0; k<NZ; k++) {
			data[k].resize(NY);
			for (int j = 0; j<NY; j++) {
				data[k][j].resize(NX);
				for (int i = 0; i<NX; i++) {

					if ((c = fgetc(fptr)) == EOF) {
						fprintf(stderr, "Unexpected end of file\n");
						exit(-1);
					}
					data[k][j][i] = c;

					if (c > themax)
						themax = c;
					if (c < themin)
						themin = c;
				}
			}
		}

		fclose(fptr);
		fprintf(stderr, "Volumetric data range: %d -> %d\n", themin, themax);

	}


	GLfloat operator()(vec3 point) {

		int z = NZ / 2 + (int)floor(point.z);
		int y = NY / 2 + (int)floor(point.y);
		int x = NX / 2 + (int)floor(point.x);

		if (z<0 || y<0 || x<0 || z>NZ || y>NY || x>NX) {
			return -999.0f;
		}
		short int level = data[z][y][x];
		return ((GLfloat)(isolevel - level));
	}


	char* label() { return "MRI"; }
};

#endif