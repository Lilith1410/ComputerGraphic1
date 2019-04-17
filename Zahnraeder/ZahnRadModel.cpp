#include "ZahnRadModel.h"

#include <cassert>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "opengl_utils.h"
#include <math.h>


// --- public member functions ---------------------------------------------------
static  double l = 0.0;			// laenge des Zahns
static   double k = 0.0;		// laenge der Rille
static   double w1 = 0.0;		// Winkel des Zahns zum Ursprung
static   double w2 = 0.0;		// Winkel der Rille zum Zahn		WICHTIG! w1 + w2 = 30 !!!!!
static 	double z = 0.0;			// Tiefe des Zahnrads

ZahnRadModel::ZahnRadModel(double _l, double _k, double _z, double _w1, double _w2) {

  init_(_l, _k, _z, _w1, _w2);
}


ZahnRadModel::~ZahnRadModel() {
  if (glfwGetCurrentContext()) {    // check for OpenGL context
    glDeleteVertexArrays(1, &vao_);
  }
}


void ZahnRadModel::render() const {
  // render model using vertex array object
  glBindVertexArray(vao_);
  glDrawElements(GL_TRIANGLES, nIndices_, GL_UNSIGNED_INT, static_cast<const GLvoid*>(0));
  glBindVertexArray(0);
}


// --- protected member functions ------------------------------------------------


void ZahnRadModel::init_(double _l, double _k, double _z, double _w1, double _w2) {
	l = _l;
	k = _k;
	z = _z;
	w1 = _w1*M_PI/180;
	w2 = _w2*M_PI/180;
  // create and bind vertex array;
  // in case of multiple objects, use a separate vertex array for each one
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);
  assert(glIsVertexArray(vao_));

   const GLfloat zahnRadVertices[] = {
		   0.0f, 0.0f, 0.0f,																	// 0

		   /*
		    * Frontseite
		    */

		   /*
		    *           |12
		    *           |   \
		    *   ________|______1
		    *           |
		    *           |
		    *           |
		    */
		(float) l, 	 0.0f, 0.0f,																// 1
		(float) (sqrt(pow(l, 2)-pow((sin(w1)*l),2))), (float) (sin(w1)*l), 	0.0f,				// 2

		(float) (sqrt(pow(k,2)-pow((sin(w1)*k),2))),	(float) (sin(w1)*k), 0.0f,				// 3
		(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	(float) (sin(w1+w2)*k), 0.0f,			// 4

		(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	(float) (sin(w1+w2)*l), 0.0f,			// 5
		(float) (sqrt(pow(l,2)-pow((sin(2*w1+w2)*l),2))),	(float) (sin(2*w1+w2)*l), 0.0f,		// 6

		(float) (sqrt(pow(k,2)-pow((sin(2*w1+w2)*k),2))),	(float) (sin(2*w1+w2)*k), 0.0f,		// 7
		(float) (sqrt(pow(k,2)-pow((sin(2*w1+2*w2)*k),2))), (float) (sin(2*w1+2*w2)*k), 0.0f,	// 8

		(float) (sqrt(pow(l,2)-pow((sin(2*w1+2*w2)*l),2))),	(float) (sin(2*w1+2*w2)*l), 0.0f,	// 9
		(float) (sqrt(pow(l,2)-pow((sin(3*w1+2*w2)*l),2))),	(float) (sin(3*w1+2*w2)*l), 0.0f,	// 10

		(float) (sqrt(pow(k,2)-pow((sin(3*w1+2*w2)*k),2))),	(float) (sin(3*w1+2*w2)*k), 0.0f,	// 11
		   0.0f, (float)k, 0.0f,																		// 12

		   /*
		    *           |
		    *           |
		    *   ________|_______
		    *           |      13
		    *           |    /
		    *           |24
		    */
		(float) k, 	 0.0f, 0.0f,																// 13
		(float) (sqrt(pow(k, 2)-pow((sin(w2)*k),2))), (float) -(sin(w2)*k), 	0.0f,			// 14

		(float) (sqrt(pow(l,2)-pow((sin(w2)*l),2))),	(float) -(sin(w2)*l), 0.0f,				// 15
		(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	(float) -(sin(w1+w2)*l), 0.0f,			// 16

		(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	(float) -(sin(w1+w2)*k), 0.0f,			// 17
		(float) (sqrt(pow(k,2)-pow((sin(2*w2+w1)*k),2))),	(float) -(sin(2*w2+w1)*k), 0.0f,	// 18

		(float) (sqrt(pow(l,2)-pow((sin(2*w2+w1)*l),2))),	(float) -(sin(2*w2+w1)*l), 0.0f,	// 19
		(float) (sqrt(pow(l,2)-pow((sin(2*w2+2*w1)*l),2))), (float) -(sin(2*w2+2*w1)*l), 0.0f,	// 20

		(float) (sqrt(pow(k,2)-pow((sin(2*w2+2*w1)*k),2))),	(float) -(sin(2*w2+2*w1)*k), 0.0f,	// 21
		(float) (sqrt(pow(k,2)-pow((sin(3*w2+2*w1)*k),2))),	(float) -(sin(3*w2+2*w1)*k), 0.0f,	// 22

		(float) (sqrt(pow(l,2)-pow((sin(3*w2+2*w1)*l),2))),	(float) -(sin(3*w2+2*w1)*l), 0.0f,	// 23
		 0.0f, (float) (-l), 0.0f,																// 24



		   /*
		    *        36 |
		    *      /    |
		    *   25______|_______
		    *           |
		    *           |
		    *           |
		    */
		-(float) k, 	 0.0f, 0.0f,															// 25
		-(float) (sqrt(pow(k, 2)-pow((sin(w2)*k),2))), (float) (sin(w2)*k), 	0.0f,			// 26

		-(float) (sqrt(pow(l,2)-pow((sin(w2)*l),2))),	(float) (sin(w2)*l), 0.0f,				// 27
		-(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	(float) (sin(w1+w2)*l), 0.0f,		// 28

		-(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	(float) (sin(w1+w2)*k), 0.0f,		// 29
		-(float) (sqrt(pow(k,2)-pow((sin(2*w2+w1)*k),2))),	(float) (sin(2*w2+w1)*k), 0.0f,		// 30

		-(float) (sqrt(pow(l,2)-pow((sin(2*w2+w1)*l),2))),	(float) (sin(2*w2+w1)*l), 0.0f,		// 31
		-(float) (sqrt(pow(l,2)-pow((sin(2*w2+2*w1)*l),2))), (float) (sin(2*w2+2*w1)*l), 0.0f,	// 32

		-(float) (sqrt(pow(k,2)-pow((sin(2*w2+2*w1)*k),2))),	(float) (sin(2*w2+2*w1)*k), 0.0f,// 33
		-(float) (sqrt(pow(k,2)-pow((sin(3*w2+2*w1)*k),2))),	(float) (sin(3*w2+2*w1)*k), 0.0f,// 34

		-(float) (sqrt(pow(l,2)-pow((sin(3*w2+2*w1)*l),2))),	(float) (sin(3*w2+2*w1)*l), 0.0f,// 35
		0.0f, (float) (l), 0.0f,																 // 36


		   /*
		    *           |
		    *           |
		    *   ________|_______
		    *   37      |
		    *      \    |
		    *         48|
		    */
		-(float) l, 	 0.0f, 0.0f,															// 37
		-(float) (sqrt(pow(l, 2)-pow((sin(w1)*l),2))), -(float) (sin(w1)*l), 	0.0f,			// 38

		-(float) (sqrt(pow(k,2)-pow((sin(w1)*k),2))),	-(float) (sin(w1)*k), 0.0f,				// 39
		-(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	-(float) (sin(w1+w2)*k), 0.0f,		// 40

		-(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	-(float) (sin(w1+w2)*l), 0.0f,		// 41
		-(float) (sqrt(pow(l,2)-pow((sin(2*w1+w2)*l),2))),	-(float) (sin(2*w1+w2)*l), 0.0f,	// 42

		-(float) (sqrt(pow(k,2)-pow((sin(2*w1+w2)*k),2))),	-(float) (sin(2*w1+w2)*k), 0.0f,	// 43
		-(float) (sqrt(pow(k,2)-pow((sin(2*w1+2*w2)*k),2))), -(float) (sin(2*w1+2*w2)*k), 0.0f,	// 44

		-(float) (sqrt(pow(l,2)-pow((sin(2*w1+2*w2)*l),2))),	-(float) (sin(2*w1+2*w2)*l), 0.0f,// 45
		-(float) (sqrt(pow(l,2)-pow((sin(3*w1+2*w2)*l),2))),	-(float) (sin(3*w1+2*w2)*l), 0.0f,// 46

		-(float) (sqrt(pow(k,2)-pow((sin(3*w1+2*w2)*k),2))),	-(float) (sin(3*w1+2*w2)*k), 0.0f,// 47
		0.0f, -(float)k, 0.0f,																	  // 48


		/*
		 * Rückseite
		 */
		   0.0f, 0.0f, (float) z,															// 49



		   /*
		    *           |61
		    *           |   \
		    *   ________|______50
		    *           |
		    *           |
		    *           |
		    */
		(float) l, 	 0.0f, (float) z,																// 50
		(float) (sqrt(pow(l, 2)-pow((sin(w1)*l),2))), (float) (sin(w1)*l), 	z,				// 51

		(float) (sqrt(pow(k,2)-pow((sin(w1)*k),2))),	(float) (sin(w1)*k), (float) z,				// 52
		(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	(float) (sin(w1+w2)*k), (float) z,			// 53

		(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	(float) (sin(w1+w2)*l), (float) z,			// 54
		(float) (sqrt(pow(l,2)-pow((sin(2*w1+w2)*l),2))),	(float) (sin(2*w1+w2)*l), (float) z,		// 55

		(float) (sqrt(pow(k,2)-pow((sin(2*w1+w2)*k),2))),	(float) (sin(2*w1+w2)*k), (float) z,		// 56
		(float) (sqrt(pow(k,2)-pow((sin(2*w1+2*w2)*k),2))), (float) (sin(2*w1+2*w2)*k), (float) z,	// 57

		(float) (sqrt(pow(l,2)-pow((sin(2*w1+2*w2)*l),2))),	(float) (sin(2*w1+2*w2)*l), (float) z,	// 58
		(float) (sqrt(pow(l,2)-pow((sin(3*w1+2*w2)*l),2))),	(float) (sin(3*w1+2*w2)*l), (float) z,	// 59

		(float) (sqrt(pow(k,2)-pow((sin(3*w1+2*w2)*k),2))),	(float) (sin(3*w1+2*w2)*k), (float) z,	// 60
		   0.0f, (float)k, (float) z,																		// 61

		   /*
		    *           |
		    *           |
		    *   ________|_______
		    *           |      62
		    *           |    /
		    *           |73
		    */
		(float) k, 	 0.0f, (float) z,																// 62
		(float) (sqrt(pow(k, 2)-pow((sin(w2)*k),2))), (float) -(sin(w2)*k), 	z,			// 63

		(float) (sqrt(pow(l,2)-pow((sin(w2)*l),2))),	(float) -(sin(w2)*l), (float) z,				// 64
		(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	(float) -(sin(w1+w2)*l), (float) z,			// 65

		(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	(float) -(sin(w1+w2)*k), (float) z,			// 66
		(float) (sqrt(pow(k,2)-pow((sin(2*w2+w1)*k),2))),	(float) -(sin(2*w2+w1)*k), (float) z,	// 67

		(float) (sqrt(pow(l,2)-pow((sin(2*w2+w1)*l),2))),	(float) -(sin(2*w2+w1)*l), (float) z,	// 68
		(float) (sqrt(pow(l,2)-pow((sin(2*w2+2*w1)*l),2))), (float) -(sin(2*w2+2*w1)*l), (float) z,	// 69

		(float) (sqrt(pow(k,2)-pow((sin(2*w2+2*w1)*k),2))),	(float) -(sin(2*w2+2*w1)*k), (float) z,	// 70
		(float) (sqrt(pow(k,2)-pow((sin(3*w2+2*w1)*k),2))),	(float) -(sin(3*w2+2*w1)*k), (float) z,	// 71

		(float) (sqrt(pow(l,2)-pow((sin(3*w2+2*w1)*l),2))),	(float) -(sin(3*w2+2*w1)*l), (float) z,	// 72
		 0.0f, (float) (-l), (float) z,																// 73


		   /*
		    *        85 |
		    *      /    |
		    *   74______|_______
		    *           |
		    *           |
		    *           |
		    */
		-(float) k, 	 0.0f, (float) z,															// 74
		-(float) (sqrt(pow(k, 2)-pow((sin(w2)*k),2))), (float) (sin(w2)*k), 	z,			// 75

		-(float) (sqrt(pow(l,2)-pow((sin(w2)*l),2))),	(float) (sin(w2)*l), (float) z,				// 76
		-(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	(float) (sin(w1+w2)*l), (float) z,		// 77

		-(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	(float) (sin(w1+w2)*k), (float) z,		// 78
		-(float) (sqrt(pow(k,2)-pow((sin(2*w2+w1)*k),2))),	(float) (sin(2*w2+w1)*k), (float) z,		// 79

		-(float) (sqrt(pow(l,2)-pow((sin(2*w2+w1)*l),2))),	(float) (sin(2*w2+w1)*l), (float) z,		// 80
		-(float) (sqrt(pow(l,2)-pow((sin(2*w2+2*w1)*l),2))), (float) (sin(2*w2+2*w1)*l), (float) z,	// 81

		-(float) (sqrt(pow(k,2)-pow((sin(2*w2+2*w1)*k),2))),	(float) (sin(2*w2+2*w1)*k), (float) z,// 82
		-(float) (sqrt(pow(k,2)-pow((sin(3*w2+2*w1)*k),2))),	(float) (sin(3*w2+2*w1)*k), (float) z,// 83

		-(float) (sqrt(pow(l,2)-pow((sin(3*w2+2*w1)*l),2))),	(float) (sin(3*w2+2*w1)*l), (float) z,// 84
		0.0f, (float) (l), (float) z,																 // 85


		   /*
		    *           |
		    *           |
		    *   ________|_______
		    *   86      |
		    *      \    |
		    *         97|
		    */
		-(float) l, 	 0.0f, (float) z,															// 86
		-(float) (sqrt(pow(l, 2)-pow((sin(w1)*l),2))), -(float) (sin(w1)*l), 	z,			// 87

		-(float) (sqrt(pow(k,2)-pow((sin(w1)*k),2))),	-(float) (sin(w1)*k), (float) z,				// 88
		-(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	-(float) (sin(w1+w2)*k), (float) z,		// 89

		-(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	-(float) (sin(w1+w2)*l), (float) z,		//90
		-(float) (sqrt(pow(l,2)-pow((sin(2*w1+w2)*l),2))),	-(float) (sin(2*w1+w2)*l), (float) z,	// 91

		-(float) (sqrt(pow(k,2)-pow((sin(2*w1+w2)*k),2))),	-(float) (sin(2*w1+w2)*k), (float) z,	// 92
		-(float) (sqrt(pow(k,2)-pow((sin(2*w1+2*w2)*k),2))), -(float) (sin(2*w1+2*w2)*k), (float) z,	// 93

		-(float) (sqrt(pow(l,2)-pow((sin(2*w1+2*w2)*l),2))),	-(float) (sin(2*w1+2*w2)*l), (float) z,// 94
		-(float) (sqrt(pow(l,2)-pow((sin(3*w1+2*w2)*l),2))),	-(float) (sin(3*w1+2*w2)*l), (float) z,// 95

		-(float) (sqrt(pow(k,2)-pow((sin(3*w1+2*w2)*k),2))),	-(float) (sin(3*w1+2*w2)*k), (float) z,// 96
		0.0f, -(float)k, (float) z																	  // 97




   };


  // create vertex VBO and enable vertex attribute
  GLuint vboVertex;
  glGenBuffers(1, &vboVertex);
  glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
  assert(glIsBuffer(vboVertex));
  glBufferData(GL_ARRAY_BUFFER, sizeof(zahnRadVertices), zahnRadVertices, GL_STATIC_DRAW);
  glVertexAttribPointer(ATTRIB_LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, static_cast<const GLvoid*>(0));
  glEnableVertexAttribArray(ATTRIB_LOC_VERTEX);

  // cube vertex colors
  const GLfloat zahnRadColors[] = {
      0.0f, 0.0f,  0.0f,
      0.0f, 0.0f,  0.0f,
      0.0f, 0.0f,  0.0f,
      0.0f, 0.0f,  0.0f };

  // create color VBO and enable color attribute
  GLuint vboColor;
  glGenBuffers(1, &vboColor);
  glBindBuffer(GL_ARRAY_BUFFER, vboColor);
  assert(glIsBuffer(vboColor));
  glBufferData(GL_ARRAY_BUFFER, sizeof(zahnRadColors), zahnRadColors, GL_STATIC_DRAW);
  glVertexAttribPointer(ATTRIB_LOC_COLOR, 3, GL_FLOAT, GL_FALSE, 0, static_cast<const GLvoid*>(0));
  glEnableVertexAttribArray(ATTRIB_LOC_COLOR);

  // cube indices (6 faces * 2 triangles -> 12 triangles)
  const GLuint zahnRadIndices[] = {

	  0, 1, 2,
      0, 3, 4,
	  0, 5, 6,
	  0, 7, 8,
	  0, 9, 10,
	  0, 11, 12,

	  0, 13, 14,
	  0, 15, 16,
	  0, 17, 18,
	  0, 19, 20,
	  0, 21, 22,
	  0, 23, 24,

	  0, 25, 26,
	  0, 27, 28,
	  0, 29, 30,
	  0, 31, 32,
	  0, 33, 34,
	  0, 35, 36,

	  0, 37, 38,
	  0, 39, 40,
	  0, 41, 42,
	  0, 43, 44,
	  0, 45, 46,
	  0, 47, 48,




	  49, 50, 51,
	  49, 52, 53,
	  49, 54, 55,
	  49, 56, 57,
	  49, 58, 59,
	  49, 60, 61,

	  49, 62, 63,
	  49, 64, 65,
	  49, 66, 67,
	  49, 68, 69,
	  49, 70, 71,
	  49, 72, 73,

	  49, 74, 75,
	  49, 76, 77,
	  49, 78, 79,
	  49, 80, 81,
	  49, 82, 83,
	  49, 84, 85,

	  49, 86, 87,
	  49, 88, 89,
	  49, 90, 91,
	  49, 92, 93,
	  49, 94, 95,
	  49, 96, 97,


	  /*
	   * Flächen um Front- und Rückseite zu verbinden
	   * Hier fehlen noch Flächen, es müssten ca 100 Einträge sein
	   */

	  1, 2, 50,
	  2, 50, 51,
	  3, 4, 52,
	  4, 52, 53,
	  5, 6, 54,
	  6, 54, 55,
	  7, 8, 56,
	  8, 56, 57,
	  9, 10, 58,
	  10, 58, 59,
	  11, 12, 60,
	  12, 60, 61,


	  13, 14, 62,
	  14, 62, 63,
	  15, 16, 64,
	  16, 64, 65,
	  17, 18, 66,
	  18, 66, 67,
	  19, 20, 68,
	  20, 68, 69,
	  21, 22, 70,
	  22, 70, 71,
	  23, 24, 72,
	  24, 72, 73,
	  25, 26, 74,
	  26, 74, 75,
	  27, 28, 76,
	  28, 76, 77,
	  29, 30, 78,
	  30, 78, 79,
	  31, 32, 80,
	  32, 80, 81,
	  33, 34, 82,
	  34, 82, 83,
	  35, 36, 84,
	  36, 84, 85,
	  37, 38, 86,
	  38, 86, 87,
	  39, 40, 88,
	  40, 88, 89,
	  41, 42, 90,
	  42, 90, 91,
	  43, 44, 92,
	  44, 92, 93,
	  45, 46, 94,
	  46, 94, 95,
	  47, 48, 96,

	  2,3,51,
	  3,51,52,
	  4,5,53,
	  5,53,54,
	  6,7,55,
	  7,55,56,
	  8,9,57,
	  9,57,58,
	  10,11,59,
	  11,59,60,
	  //12,13,61,
	  //13,61,62,
	  14,15,63,
	  15,63,64,
	  16,17,65,
	  17,65,66,
	  18,19,67,
	  19,67,68,
	  20,21,69,
	  21,69,70,
	  22,23,71,
	  23,71,72,
//	  24,25,73,
//	  25,73,74,
	  26,27,75,
	  27,75,76,
	  28,29,77,
	  29,77,78,
	  30,31,79,
	  31,79,80,
	  32,33,81,
	  33,81,82,
	  34,35,83,
	  35,83,84,
	  //36,37,85,
	  //37,85,86,
	  38,39,87,
	  39,87,88,
	  40,41,89,
	  41,89,90,
	  42,43,91,
	  43,91,92,
	  44,45,93,
	  45,93,94,
	  46,47,95,
	  47,95,96,
	  48,49,97


  	  };

  // create index VBO
  GLuint vboIndex;
  glGenBuffers(1, &vboIndex);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndex);
  assert(glIsBuffer(vboIndex));
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(zahnRadIndices), zahnRadIndices, GL_STATIC_DRAW);
  nIndices_ = sizeof(zahnRadIndices) / sizeof(GLuint);

  // unbind vertex array
  glBindVertexArray(0);

  assert(!checkGLError_());
}
