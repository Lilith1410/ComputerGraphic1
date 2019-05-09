/**
 * \file GeometryCoreFactory.cpp
 *
 * \author Volker Ahlers\n
 *         volker.ahlers@hs-hannover.de
 */

/*
 * Copyright 2014-2019 Volker Ahlers
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cfloat>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "GeometryCore.h"
#include "GeometryCoreFactory.h"
#include "scg_utilities.h"

namespace scg {


GeometryCoreFactory::GeometryCoreFactory() {
}


GeometryCoreFactory::GeometryCoreFactory(const std::string& filePath){
  addFilePath(filePath);
}


GeometryCoreFactory::~GeometryCoreFactory() {
}


void GeometryCoreFactory::addFilePath(const std::string& filePath) {
  assert(!filePath.empty());
  splitFilePath(filePath, filePaths_);
}


GeometryCoreSP GeometryCoreFactory::createModelFromOBJFile(const std::string& fileName) {

  // read OBJ model
  OBJModel model;
  int error = loadOBJFile_(fileName, model);
  if (error != 0) {
    throw std::runtime_error("cannot open file " + fileName
        + " [GeometryCoreFactory::createModelFromOBJFile()]");
  }

  // create geometry core
  auto core = GeometryCore::create(GL_TRIANGLES, DrawMode::ARRAYS);
  GLfloat* vertices = new GLfloat[3 * 3 * model.nTriangles];
  GLfloat* normals = new GLfloat[3 * 3 * model.nTriangles];
  GLfloat* texCoords = nullptr;
  if (!model.texCoords.empty()) {
    texCoords = new GLfloat[2 * 3 * model.nTriangles];
  }
  int vertIdx = 0;
  int texIdx = 0;
  for (auto face : model.faces) {
    for (int i = 0; i < face.nTriangles; ++i) {
      glm::vec3 faceNormal = glm::normalize(glm::cross(
          (model.vertices[face.entries[i + 1].vertex - 1] - model.vertices[face.entries[0].vertex - 1]),
          (model.vertices[face.entries[i + 2].vertex - 1] - model.vertices[face.entries[0].vertex - 1])));
      memcpy(&vertices[vertIdx], glm::value_ptr(model.vertices[face.entries[0].vertex - 1]), 3 * sizeof(GLfloat));
      if (face.entries[0].normal != 0) {
        memcpy(&normals[vertIdx], glm::value_ptr(model.normals[face.entries[0].normal - 1]), 3 * sizeof(GLfloat));
      }
      else {
        memcpy(&normals[vertIdx], glm::value_ptr(faceNormal), 3 * sizeof(GLfloat));
      }
      vertIdx += 3;
      if (texCoords) {
        if (face.entries[0].texCoord != 0) {
          memcpy(&texCoords[texIdx], glm::value_ptr(model.texCoords[face.entries[0].texCoord - 1]), 2 * sizeof(GLfloat));
        }
        else {
          memcpy(&texCoords[texIdx], glm::value_ptr(glm::vec2()), 2 * sizeof(GLfloat));
        }
        texIdx += 2;
      }
      for (int j = 1; j <= 2; ++j) {
        memcpy(&vertices[vertIdx], glm::value_ptr(model.vertices[face.entries[i + j].vertex - 1]), 3 * sizeof(GLfloat));
        if (face.entries[i + j].normal != 0) {
          memcpy(&normals[vertIdx], glm::value_ptr(model.normals[face.entries[i + j].normal - 1]), 3 * sizeof(GLfloat));
        }
        else {
          memcpy(&normals[vertIdx], glm::value_ptr(faceNormal), 3 * sizeof(GLfloat));
        }
        vertIdx += 3;
        if (texCoords) {
          if (face.entries[i + j].texCoord != 0) {
            memcpy(&texCoords[texIdx], glm::value_ptr(model.texCoords[face.entries[i + j].texCoord - 1]), 2 * sizeof(GLfloat));
          }
          else {
            memcpy(&texCoords[texIdx], glm::value_ptr(glm::vec2()), 2 * sizeof(GLfloat));
          }
          texIdx += 2;
        }
      }
    }
  }
  core->addAttributeData(OGLConstants::VERTEX.location, vertices,
      3 * 3 * model.nTriangles * sizeof(GLfloat), 3, GL_STATIC_DRAW);
  core->addAttributeData(OGLConstants::NORMAL.location, normals,
      3 * 3 * model.nTriangles * sizeof(GLfloat), 3, GL_STATIC_DRAW);
  if (texCoords) {
    core->addAttributeData(OGLConstants::TEX_COORD_0.location, texCoords,
        2 * 3 * model.nTriangles * sizeof(GLfloat), 2, GL_STATIC_DRAW);
  }
  delete [] vertices;
  vertices = nullptr;
  delete [] texCoords;
  texCoords = nullptr;
  delete [] normals;
  normals = nullptr;

  return core;
}


/*
 * createTest
 */
GeometryCoreSP GeometryCoreFactory::createTest() {
  // create geometry core
  auto core = GeometryCore::create(GL_TRIANGLES, DrawMode::ELEMENTS);
  // define vertices
  GLfloat vertices[] = {
		  0.5f, 0.5f, 0.0f,
		  -0.5f, 0.5f, 0.0f,
		  -0.5f, -0.5f, 0.0f,
		  0.5f, -0.5f, 0.0f,

  };
/*
  // scale and translate vertices to unit size and origin center
  GLfloat factor = 1.0f / 50.f; //GLfloat factor = size / 50.f;
  int nVertices = sizeof(vertices) / sizeof(GLfloat);
  for (int i = 0; i < nVertices; ++i) {
    vertices[i] *= factor;
  }
  for (int i = 2; i < nVertices; i += 3) {
    vertices[i] -= 0.4f;
  } */
  core->addAttributeData(OGLConstants::VERTEX.location, vertices,
      sizeof(vertices), 3, GL_STATIC_DRAW);


  //define indices
  const GLuint indices[] = {
		 0,1,3,
		 1,2,3
  };
  core->setElementIndexData(indices, sizeof(indices), GL_STATIC_DRAW);


  return core;
}




/*
*
* createGear
*
* l: length of the long part
* k: length of the short part
* z: width of the gear
* w1: angle of the long part
* w2: angle of the short part
* !! (w1 + w2 = 30°)
*
*GeometryCoreSP GeometryCoreFactory::createGear(GLfloat size) {
*/
GeometryCoreSP GeometryCoreFactory::createGear(double l, double k, double z, double _w1, double _w2) {
  // create geometry core
  auto core = GeometryCore::create(GL_TRIANGLES, DrawMode::ELEMENTS);
  GLfloat z_vorn = z/2;
  GLfloat z_hinten = -z/2;
  double w1 = _w1*M_PI/180;
  double w2 = _w2*M_PI/180;
  // define vertices
  GLfloat vertices[] = {
		   0.0f, 0.0f, z_vorn,																	// 0

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
		(float) l, 	 0.0f, z_vorn,																// 1
		(float) (sqrt(pow(l, 2)-pow((sin(w1)*l),2))), (float) (sin(w1)*l), 	z_vorn,				// 2

		(float) (sqrt(pow(k,2)-pow((sin(w1)*k),2))),	(float) (sin(w1)*k), z_vorn,				// 3
		(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	(float) (sin(w1+w2)*k), z_vorn,			// 4

		(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	(float) (sin(w1+w2)*l), z_vorn,			// 5
		(float) (sqrt(pow(l,2)-pow((sin(2*w1+w2)*l),2))),	(float) (sin(2*w1+w2)*l), z_vorn,		// 6

		(float) (sqrt(pow(k,2)-pow((sin(2*w1+w2)*k),2))),	(float) (sin(2*w1+w2)*k), z_vorn,		// 7
		(float) (sqrt(pow(k,2)-pow((sin(2*w1+2*w2)*k),2))), (float) (sin(2*w1+2*w2)*k), z_vorn,	// 8

		(float) (sqrt(pow(l,2)-pow((sin(2*w1+2*w2)*l),2))),	(float) (sin(2*w1+2*w2)*l), z_vorn,	// 9
		(float) (sqrt(pow(l,2)-pow((sin(3*w1+2*w2)*l),2))),	(float) (sin(3*w1+2*w2)*l), z_vorn,	// 10

		(float) (sqrt(pow(k,2)-pow((sin(3*w1+2*w2)*k),2))),	(float) (sin(3*w1+2*w2)*k), z_vorn,	// 11
		   0.0f, (float)k, z_vorn,												 				// 12

		   /*
		    *           |
		    *           |
		    *   ________|_______
		    *           |      13
		    *           |    /
		    *           |24
		    */
		(float) k, 	 0.0f, z_vorn,																// 13
		(float) (sqrt(pow(k, 2)-pow((sin(w2)*k),2))), (float) -(sin(w2)*k), 	z_vorn,			// 14

		(float) (sqrt(pow(l,2)-pow((sin(w2)*l),2))),	(float) -(sin(w2)*l), z_vorn,				// 15
		(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	(float) -(sin(w1+w2)*l), z_vorn,			// 16

		(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	(float) -(sin(w1+w2)*k), z_vorn,			// 17
		(float) (sqrt(pow(k,2)-pow((sin(2*w2+w1)*k),2))),	(float) -(sin(2*w2+w1)*k), z_vorn,	// 18

		(float) (sqrt(pow(l,2)-pow((sin(2*w2+w1)*l),2))),	(float) -(sin(2*w2+w1)*l), z_vorn,	// 19
		(float) (sqrt(pow(l,2)-pow((sin(2*w2+2*w1)*l),2))), (float) -(sin(2*w2+2*w1)*l), z_vorn,	// 20

		(float) (sqrt(pow(k,2)-pow((sin(2*w2+2*w1)*k),2))),	(float) -(sin(2*w2+2*w1)*k), z_vorn,	// 21
		(float) (sqrt(pow(k,2)-pow((sin(3*w2+2*w1)*k),2))),	(float) -(sin(3*w2+2*w1)*k), z_vorn,	// 22

		(float) (sqrt(pow(l,2)-pow((sin(3*w2+2*w1)*l),2))),	(float) -(sin(3*w2+2*w1)*l), z_vorn,	// 23
		 0.0f, (float) (-l), z_vorn,																// 24



		   /*
		    *        36 |
		    *      /    |
		    *   25______|_______
		    *           |
		    *           |
		    *           |
		    */
		-(float) k, 	 0.0f, z_vorn,															// 25
		-(float) (sqrt(pow(k, 2)-pow((sin(w2)*k),2))), (float) (sin(w2)*k), 	z_vorn,			// 26

		-(float) (sqrt(pow(l,2)-pow((sin(w2)*l),2))),	(float) (sin(w2)*l), z_vorn,				// 27
		-(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	(float) (sin(w1+w2)*l), z_vorn,		// 28

		-(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	(float) (sin(w1+w2)*k), z_vorn,		// 29
		-(float) (sqrt(pow(k,2)-pow((sin(2*w2+w1)*k),2))),	(float) (sin(2*w2+w1)*k), z_vorn,		// 30

		-(float) (sqrt(pow(l,2)-pow((sin(2*w2+w1)*l),2))),	(float) (sin(2*w2+w1)*l), z_vorn,		// 31
		-(float) (sqrt(pow(l,2)-pow((sin(2*w2+2*w1)*l),2))), (float) (sin(2*w2+2*w1)*l), z_vorn,	// 32

		-(float) (sqrt(pow(k,2)-pow((sin(2*w2+2*w1)*k),2))),	(float) (sin(2*w2+2*w1)*k), z_vorn,// 33
		-(float) (sqrt(pow(k,2)-pow((sin(3*w2+2*w1)*k),2))),	(float) (sin(3*w2+2*w1)*k), z_vorn,// 34

		-(float) (sqrt(pow(l,2)-pow((sin(3*w2+2*w1)*l),2))),	(float) (sin(3*w2+2*w1)*l), z_vorn,// 35
		0.0f, (float) (l), z_vorn,																 // 36


		   /*
		    *           |
		    *           |
		    *   ________|_______
		    *   37      |
		    *      \    |
		    *         48|
		    */
		-(float) l, 	 0.0f, z_vorn,															// 37
		-(float) (sqrt(pow(l, 2)-pow((sin(w1)*l),2))), -(float) (sin(w1)*l), z_vorn,			// 38

		-(float) (sqrt(pow(k,2)-pow((sin(w1)*k),2))),	-(float) (sin(w1)*k), z_vorn,				// 39
		-(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	-(float) (sin(w1+w2)*k), z_vorn,		// 40

		-(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	-(float) (sin(w1+w2)*l), z_vorn,		// 41
		-(float) (sqrt(pow(l,2)-pow((sin(2*w1+w2)*l),2))),	-(float) (sin(2*w1+w2)*l), z_vorn,	// 42

		-(float) (sqrt(pow(k,2)-pow((sin(2*w1+w2)*k),2))),	-(float) (sin(2*w1+w2)*k), z_vorn,	// 43
		-(float) (sqrt(pow(k,2)-pow((sin(2*w1+2*w2)*k),2))), -(float) (sin(2*w1+2*w2)*k), z_vorn,	// 44

		-(float) (sqrt(pow(l,2)-pow((sin(2*w1+2*w2)*l),2))),	-(float) (sin(2*w1+2*w2)*l), z_vorn,// 45
		-(float) (sqrt(pow(l,2)-pow((sin(3*w1+2*w2)*l),2))),	-(float) (sin(3*w1+2*w2)*l), z_vorn,// 46

		-(float) (sqrt(pow(k,2)-pow((sin(3*w1+2*w2)*k),2))),	-(float) (sin(3*w1+2*w2)*k), z_vorn,// 47
		0.0f, -(float)k, z_vorn,																	  // 48


		/*
		 * Rückseite
		 */
		   0.0f, 0.0f, (float) z_hinten,																// 49



		   /*
		    *           |61
		    *           |   \
		    *   ________|______50
		    *           |
		    *           |
		    *           |
		    */
		(float) l, 	 0.0f, (float) z_hinten,																// 50
		(float) (sqrt(pow(l, 2)-pow((sin(w1)*l),2))), (float) (sin(w1)*l), 	z_hinten,						// 51

		(float) (sqrt(pow(k,2)-pow((sin(w1)*k),2))),	(float) (sin(w1)*k), (float) z_hinten,				// 52
		(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	(float) (sin(w1+w2)*k), (float) z_hinten,			// 53

		(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	(float) (sin(w1+w2)*l), (float) z_hinten,			// 54
		(float) (sqrt(pow(l,2)-pow((sin(2*w1+w2)*l),2))),	(float) (sin(2*w1+w2)*l), (float) z_hinten,	// 55

		(float) (sqrt(pow(k,2)-pow((sin(2*w1+w2)*k),2))),	(float) (sin(2*w1+w2)*k), (float) z_hinten,	// 56
		(float) (sqrt(pow(k,2)-pow((sin(2*w1+2*w2)*k),2))), (float) (sin(2*w1+2*w2)*k), (float) z_hinten,	// 57

		(float) (sqrt(pow(l,2)-pow((sin(2*w1+2*w2)*l),2))),	(float) (sin(2*w1+2*w2)*l), (float) z_hinten,	// 58
		(float) (sqrt(pow(l,2)-pow((sin(3*w1+2*w2)*l),2))),	(float) (sin(3*w1+2*w2)*l), (float) z_hinten,	// 59

		(float) (sqrt(pow(k,2)-pow((sin(3*w1+2*w2)*k),2))),	(float) (sin(3*w1+2*w2)*k), (float) z_hinten,	// 60
		   0.0f, (float)k, (float) z_hinten,																// 61

		   /*
		    *           |
		    *           |
		    *   ________|_______
		    *           |      62
		    *           |    /
		    *           |73
		    */
		(float) k, 	 0.0f, (float) z_hinten,																// 62
		(float) (sqrt(pow(k, 2)-pow((sin(w2)*k),2))), (float) -(sin(w2)*k), z_hinten,					// 63

		(float) (sqrt(pow(l,2)-pow((sin(w2)*l),2))),	(float) -(sin(w2)*l), (float) z_hinten,			// 64
		(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	(float) -(sin(w1+w2)*l), (float) z_hinten,			// 65

		(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	(float) -(sin(w1+w2)*k), (float) z_hinten,			// 66
		(float) (sqrt(pow(k,2)-pow((sin(2*w2+w1)*k),2))),	(float) -(sin(2*w2+w1)*k), (float) z_hinten,	// 67

		(float) (sqrt(pow(l,2)-pow((sin(2*w2+w1)*l),2))),	(float) -(sin(2*w2+w1)*l), (float) z_hinten,	// 68
		(float) (sqrt(pow(l,2)-pow((sin(2*w2+2*w1)*l),2))), (float) -(sin(2*w2+2*w1)*l), (float) z_hinten,	// 69

		(float) (sqrt(pow(k,2)-pow((sin(2*w2+2*w1)*k),2))),	(float) -(sin(2*w2+2*w1)*k), (float) z_hinten,	// 70
		(float) (sqrt(pow(k,2)-pow((sin(3*w2+2*w1)*k),2))),	(float) -(sin(3*w2+2*w1)*k), (float) z_hinten,	// 71

		(float) (sqrt(pow(l,2)-pow((sin(3*w2+2*w1)*l),2))),	(float) -(sin(3*w2+2*w1)*l), (float) z_hinten,	// 72
		 0.0f, (float) (-l), (float) z_hinten,																// 73


		   /*
		    *        85 |
		    *      /    |
		    *   74______|_______
		    *           |
		    *           |
		    *           |
		    */
		-(float) k, 	 0.0f, (float) z_hinten,															// 74
		-(float) (sqrt(pow(k, 2)-pow((sin(w2)*k),2))), (float) (sin(w2)*k), (float)	z_hinten,					// 75

		-(float) (sqrt(pow(l,2)-pow((sin(w2)*l),2))),	(float) (sin(w2)*l), (float) z_hinten,				// 76
		-(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	(float) (sin(w1+w2)*l), (float) z_hinten,		// 77

		-(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	(float) (sin(w1+w2)*k), (float) z_hinten,		// 78
		-(float) (sqrt(pow(k,2)-pow((sin(2*w2+w1)*k),2))),	(float) (sin(2*w2+w1)*k), (float) z_hinten,	// 79

		-(float) (sqrt(pow(l,2)-pow((sin(2*w2+w1)*l),2))),	(float) (sin(2*w2+w1)*l), (float) z_hinten,	// 80
		-(float) (sqrt(pow(l,2)-pow((sin(2*w2+2*w1)*l),2))), (float) (sin(2*w2+2*w1)*l), (float) z_hinten,	// 81

		-(float) (sqrt(pow(k,2)-pow((sin(2*w2+2*w1)*k),2))),	(float) (sin(2*w2+2*w1)*k), (float) z_hinten,// 82
		-(float) (sqrt(pow(k,2)-pow((sin(3*w2+2*w1)*k),2))),	(float) (sin(3*w2+2*w1)*k), (float) z_hinten,// 83

		-(float) (sqrt(pow(l,2)-pow((sin(3*w2+2*w1)*l),2))),	(float) (sin(3*w2+2*w1)*l), (float) z_hinten,// 84
		0.0f, (float) (l), (float) z_hinten,																 // 85


		   /*
		    *           |
		    *           |
		    *   ________|_______
		    *   86      |
		    *      \    |
		    *         97|
		    */
		-(float) l, 	 0.0f, (float) z_hinten,															// 86
		-(float) (sqrt(pow(l, 2)-pow((sin(w1)*l),2))), -(float) (sin(w1)*l), 	z_hinten,					// 87

		-(float) (sqrt(pow(k,2)-pow((sin(w1)*k),2))),	-(float) (sin(w1)*k), (float) z_hinten,			// 88
		-(float) (sqrt(pow(k,2)-pow((sin(w1+w2)*k),2))),	-(float) (sin(w1+w2)*k), (float) z_hinten,		// 89

		-(float) (sqrt(pow(l,2)-pow((sin(w1+w2)*l),2))),	-(float) (sin(w1+w2)*l), (float) z_hinten,		//90
		-(float) (sqrt(pow(l,2)-pow((sin(2*w1+w2)*l),2))),	-(float) (sin(2*w1+w2)*l), (float) z_hinten,	// 91

		-(float) (sqrt(pow(k,2)-pow((sin(2*w1+w2)*k),2))),	-(float) (sin(2*w1+w2)*k), (float) z_hinten,	// 92
		-(float) (sqrt(pow(k,2)-pow((sin(2*w1+2*w2)*k),2))), -(float) (sin(2*w1+2*w2)*k), (float) z_hinten,// 93

		-(float) (sqrt(pow(l,2)-pow((sin(2*w1+2*w2)*l),2))),	-(float) (sin(2*w1+2*w2)*l), (float) z_hinten,// 94
		-(float) (sqrt(pow(l,2)-pow((sin(3*w1+2*w2)*l),2))),	-(float) (sin(3*w1+2*w2)*l), (float) z_hinten,// 95

		-(float) (sqrt(pow(k,2)-pow((sin(3*w1+2*w2)*k),2))),	-(float) (sin(3*w1+2*w2)*k), (float) z_hinten,// 96
		0.0f, -(float)k, (float) z_hinten													  // 97

  };
/*
  // scale and translate vertices to unit size and origin center
  GLfloat factor = 1.0f / 50.f; //GLfloat factor = size / 50.f;
  int nVertices = sizeof(vertices) / sizeof(GLfloat);
  for (int i = 0; i < nVertices; ++i) {
    vertices[i] *= factor;
  }
  for (int i = 2; i < nVertices; i += 3) {
    vertices[i] -= 0.4f;
  } */
  core->addAttributeData(OGLConstants::VERTEX.location, vertices,
      sizeof(vertices), 3, GL_STATIC_DRAW);


  //define indices
  const GLuint indices[] = {
// Frontseite, rechtsläufig verbunden
    /*
     *           |12
     *           |   \
     *   ________|______1
     *           |
     *           |
     *           |
	 */
		  0,12,11,
		  0,10,9,
		  0,8,7,
		  0,6,5,
		  0,4,3,
		  0,2,1,

   /*
    *           |
    *           |
    *   ________|_______
    *           |      13
    *           |    /
    *           |24
    */
		  0,13,14,
		  0,15,16,
		  0,17,18,
		  0,19,20,
		  0,21,22,
		  0,23,24,

   /*
    *           |
    *           |
    *   ________|_______
    *   37      |
    *      \    |
    *         48|
	*/
		  0,48,47,
		  0,46,45,
		  0,44,43,
		  0,42,41,
		  0,40,39,
		  0,38,37,


   /*
    *        36 |
    *      /    |
    *   25______|_______
    *           |
    *           |
    *           |
    */
		  0,25,26,
		  0,27,28,
		  0,29,30,
		  0,31,32,
		  0,33,34,
		  0,35,36,


// Rückseite, rechtsläufig verbunden
	 /*
	  *           |61
	  *           |   \
	  *   ________|______50
	  *           |
	  *           |
	  *           |
	  */
		  61,60,0,
		  59,58,0,
		  57,56,0,
		  55,54,0,
		  53,52,0,
		  51,50,0,

	 /*
	  *           |
	  *           |
	  *   ________|_______
	  *           |      62
	  *           |    /
	  *           |73
	  */
		  62,63,0,
		  64,65,0,
		  66,67,0,
		  68,69,0,
		  70,71,0,
		  72,73,0,

	 /*
	  *           |
	  *           |
	  *   ________|_______
	  *   86      |
	  *      \    |
	  *         97|
	  */
		  97,96,0,
		  95,94,0,
		  93,92,0,
		  91,90,0,
		  89,88,0,
		  87,86,0,

	   /*
		*        85 |
		*      /    |
		*   74______|_______
		*           |
		*           |
		*           |
		*/

		  74,75,0,
		  76,77,0,
		  78,79,0,
		  80,81,0,
		  82,83,0,
		  84,85,0,

		  //Seitenflächen

      /*
	   *           |12 + 61
	   *           |   \
	   *   ________|______1 + 50
	   *           |
	   *           |
	   *           |
	   */

		  12,11,60,
		  12,61,60,
		  11,10,59,
		  11,60,59,
		  10,9,58,
		  10,59,58,
		  9,8,57,
		  9,58,57,
		  8,7,56,
		  8,57,56,
		  7,6,55,
		  7,56,55,
		  6,5,54,
		  6,55,54,
		  5,4,53,
		  5,54,53,
		  4,3,52,
		  4,53,52,
		  3,2,51,
		  3,52,51,
		  2,1,50,
		  2,51,50,
		  1,13,62,
		  1,50,62,

     /*
	  *           |
	  *           |
	  *   ________|_______
	  *           |      13 + 62
	  *           |    /
	  *           |24 + 73
	  */

		  13,14,63,
		  13,62,63,
		  14,15,64,
		  14,63,64,
		  15,16,65,
		  15,64,65,
		  16,17,66,
		  16,65,66,
		  17,18,67,
		  17,66,67,
		  18,19,68,
		  18,67,68,
		  19,20,69,
		  19,68,69,
		  20,21,70,
		  20,69,70,
		  21,22,71,
		  21,70,71,
		  22,23,72,
		  22,71,72,
		  23,24,73,
		  23,72,73,
		  24,48,97,
		  24,73,97,

     /*
	  *           |
	  *           |
	  *   ________|_______
	  *37 + 86    |
	  *      \    |
	  *    48 + 97|
	  */



		  48,47,96,
		  48,97,96,
		  47,46,95,
		  47,96,95,
		  46,45,94,
		  46,95,94,
		  45,44,93,
		  45,94,93,
		  44,43,92,
		  44,93,92,
		  43,42,91,
		  43,92,91,
		  42,41,90,
		  42,91,90,
		  41,40,89,
		  41,90,89,
		  40,39,88,
		  40,89,88,
		  39,38,87,
		  39,88,87,
		  38,37,86,
		  38,87,86,
		  37,25,74,
		  37,86,74,

     /*
	  *   36 + 85 |
	  *      /    |
	  *25 + 74____|_______
	  *           |
	  *           |
	  *           |
	  */
		  25,26,75,
		  25,74,75,
		  26,27,76,
		  26,75,76,
		  27,28,77,
		  27,76,77,
		  28,29,78,
		  28,77,78,
		  29,30,79,
		  29,78,79,
		  30,31,80,
		  30,79,80,
		  31,32,81,
		  31,80,81,
		  32,33,82,
		  32,81,82,
		  33,34,83,
		  33,82,83,
		  34,35,84,
		  34,83,84,
		  35,36,85,
		  35,84,85,
		  36,12,61,
		  36,85,61



		/*  0, 1, 2,
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
		  48,49,97 */
  };
  core->setElementIndexData(indices, sizeof(indices), GL_STATIC_DRAW);



//  /*
//   * start: normals berechnen
//   */
//
  // anzahl der Dreiecke herausfinden
  int triangles_num = (sizeof(indices) / sizeof(int)) / 3;

  // Arraygröße für die normalen festlegen ( Anzahl der Dreiecke * 3, da jeder Punkt x,y und z hat)
  GLfloat normals[triangles_num*3];

  // jedes Dreieck aufrufen
  for(int i=0; i<triangles_num; i++){

	  // Indices eines Dreiecks auslesen
	  int indice_p1 = indices[i*3];
	  int indice_p2 = indices[(i*3)+1];
	  int indice_p3 = indices[(i*3)+2];

	  // get Triangle vertices anhand der Indices
	  int vx1 = indice_p1*3;	// stelle des x Wertes im Array
	  int vy1 = vx1+1;			// stelle des y Wertes im Array = x + 1
	  int vz1 = vy1+1;			// stelle des z Wertes im Array = y + 1, oder x + 2

	  int vx2 = indice_p2*3;
	  int vy2 = vx2+1;
	  int vz2 = vy2+1;

	  int vx3 = indice_p3*3;
	  int vy3 = vx3+1;
	  int vz3 = vy3+1;

	  // Kantenvektoren berechnen
	  float kanteA[] = {vertices[vx2]-vertices[vx1], vertices[vy2]-vertices[vy1], vertices[vz2]-vertices[vz1]}; // Vektor2-Vektor1
	  float kanteB[] = {vertices[vx3]-vertices[vx1], vertices[vy3]-vertices[vy1], vertices[vz3]-vertices[vz1]}; // Vektor3-Vektor1

	  // Normalenvaktor berechnen kanteA x KanteB
	  float nVec[] = {	(kanteA[1]*kanteB[2])-(kanteA[2]*kanteB[1]),
			  	  	  	  (kanteA[2]*kanteB[0])-(kanteA[0]*kanteB[2]),
						  (kanteA[0]*kanteB[1])-(kanteA[1]*kanteB[0])
	  };

	  // Normierung
	  float nVec_length = sqrt(pow(nVec[0],2)+pow(nVec[1],2)+pow(nVec[2],2)); // länge des Normalvektor
	  float normVec[] = { (nVec[0]/nVec_length),
			  	  	  	  (nVec[1]/nVec_length),
						  (nVec[2]/nVec_length)

	  };

	  //set normals
	  normals[(i*3)] = normVec[0];
	  normals[(i*3)+1] = normVec[1];
	  normals[(i*3)+2] = normVec[2];
  };

  core->addAttributeData(OGLConstants::NORMAL.location, normals,
      sizeof(normals), 3, GL_STATIC_DRAW);

  //  /*
  //   * ende: normals berechnen
  //   */
  //




  /*
  // define tangents
  const GLfloat tangents[] = {
      0.012897999957203865,
  };
  core->addAttributeData(OGLConstants::TANGENT.location, tangents,
      sizeof(tangents), 3, GL_STATIC_DRAW);

  // define binormals
  const GLfloat binormals[] = {
      0.2554270029067993, 
  };
  core->addAttributeData(OGLConstants::BINORMAL.location, binormals,
      sizeof(binormals), 3, GL_STATIC_DRAW);

  // define texture coordinates (3D)
  GLfloat texCoords[] = {
      2, 2, 0, 
  };


  // mirror texture coordinates
  int nTexCoords = sizeof(texCoords) / sizeof(GLfloat);
  for (int i = 1; i < nTexCoords; i += 3) {
    texCoords[i] = 2.f - texCoords[i];
  }
  core->addAttributeData(OGLConstants::TEX_COORD_0.location, texCoords,
      sizeof(texCoords), 3, GL_STATIC_DRAW);
*/

  return core;
}










GeometryCoreSP GeometryCoreFactory::createRectangle(glm::vec2 size) {
  // create geometry core
  auto core = GeometryCore::create(GL_TRIANGLES, DrawMode::ELEMENTS);

  // define vertices (rectangle area = size.x x size.y,
  //   4 vertices with indices 0,...,3)
  glm::vec2 halfSize = 0.5f * size;
  const GLfloat vertices[] = {
      -halfSize.x, -halfSize.y,  0.f,
       halfSize.x, -halfSize.y,  0.f,
       halfSize.x,  halfSize.y,  0.f,
      -halfSize.x,  halfSize.y,  0.f
  };
  core->addAttributeData(OGLConstants::VERTEX.location, vertices,
      sizeof(vertices), 3, GL_STATIC_DRAW);

  // define normals
  const GLfloat normals[] = {
      0.f, 0.f, 1.f,
      0.f, 0.f, 1.f,
      0.f, 0.f, 1.f,
      0.f, 0.f, 1.f
  };
  core->addAttributeData(OGLConstants::NORMAL.location, normals,
      sizeof(normals), 3, GL_STATIC_DRAW);

  // define tangents
  const GLfloat tangents[] = {
      1.f, 0.f, 0.f,
      1.f, 0.f, 0.f,
      1.f, 0.f, 0.f,
      1.f, 0.f, 0.f
  };
  core->addAttributeData(OGLConstants::TANGENT.location, tangents,
      sizeof(tangents), 3, GL_STATIC_DRAW);

  // define binormals
  const GLfloat binormals[] = {
      0.f, 1.f, 0.f,
      0.f, 1.f, 0.f,
      0.f, 1.f, 0.f,
      0.f, 1.f, 0.f
  };
  core->addAttributeData(OGLConstants::BINORMAL.location, binormals,
      sizeof(binormals), 3, GL_STATIC_DRAW);

  // define texture coordinates
  const GLfloat texCoords[] = {
       0.f, 0.f,
       1.f, 0.f,
       1.f, 1.f,
       0.f, 1.f
  };
  core->addAttributeData(OGLConstants::TEX_COORD_0.location, texCoords,
      sizeof(texCoords), 2, GL_STATIC_DRAW);

  // define indices (1 face * 2 triangles -> 2 triangles)
  const GLuint indices[] = {
      0, 1, 3,
      1, 2, 3
  };
  core->setElementIndexData(indices, sizeof(indices), GL_STATIC_DRAW);

  return core;
}


GeometryCoreSP GeometryCoreFactory::createCube(GLfloat size) {
  return createCuboid(glm::vec3(size, size, size));
}


GeometryCoreSP GeometryCoreFactory::createCuboid(glm::vec3 size) {
  // create geometry core
  auto core = GeometryCore::create(GL_TRIANGLES, DrawMode::ELEMENTS);

  // define vertices (cuboid volume = size.x * size.y * size.z,
  //   6 * 4 = 24 vertices)
  glm::vec3 halfSize = 0.5f * size;
  const GLfloat vertices[] = {
      -halfSize.x, -halfSize.y,  halfSize.z,  // front face
       halfSize.x, -halfSize.y,  halfSize.z,
       halfSize.x,  halfSize.y,  halfSize.z,
      -halfSize.x,  halfSize.y,  halfSize.z,
       halfSize.x, -halfSize.y, -halfSize.z,  // rear face
      -halfSize.x, -halfSize.y, -halfSize.z,
      -halfSize.x,  halfSize.y, -halfSize.z,
       halfSize.x,  halfSize.y, -halfSize.z,
      -halfSize.x, -halfSize.y, -halfSize.z,  // left face
      -halfSize.x, -halfSize.y,  halfSize.z,
      -halfSize.x,  halfSize.y,  halfSize.z,
      -halfSize.x,  halfSize.y, -halfSize.z,
       halfSize.x, -halfSize.y,  halfSize.z,  // right face
       halfSize.x, -halfSize.y, -halfSize.z,
       halfSize.x,  halfSize.y, -halfSize.z,
       halfSize.x,  halfSize.y,  halfSize.z,
      -halfSize.x,  halfSize.y,  halfSize.z,  // top face
       halfSize.x,  halfSize.y,  halfSize.z,
       halfSize.x,  halfSize.y, -halfSize.z,
      -halfSize.x,  halfSize.y, -halfSize.z,
      -halfSize.x, -halfSize.y, -halfSize.z,  // bottom face
       halfSize.x, -halfSize.y, -halfSize.z,
       halfSize.x, -halfSize.y,  halfSize.z,
      -halfSize.x, -halfSize.y,  halfSize.z
  };
  core->addAttributeData(OGLConstants::VERTEX.location, vertices,
      sizeof(vertices), 3, GL_STATIC_DRAW);

  // define normals
  const GLfloat normals[] = {
       0.f,  0.f,  1.f,  // front face
       0.f,  0.f,  1.f,
       0.f,  0.f,  1.f,
       0.f,  0.f,  1.f,
       0.f,  0.f, -1.f,  // rear face ...
       0.f,  0.f, -1.f,
       0.f,  0.f, -1.f,
       0.f,  0.f, -1.f,
      -1.f,  0.f,  0.f,
      -1.f,  0.f,  0.f,
      -1.f,  0.f,  0.f,
      -1.f,  0.f,  0.f,
       1.f,  0.f,  0.f,
       1.f,  0.f,  0.f,
       1.f,  0.f,  0.f,
       1.f,  0.f,  0.f,
       0.f,  1.f,  0.f,
       0.f,  1.f,  0.f,
       0.f,  1.f,  0.f,
       0.f,  1.f,  0.f,
       0.f, -1.f,  0.f,
       0.f, -1.f,  0.f,
       0.f, -1.f,  0.f,
       0.f, -1.f,  0.f
  };
  core->addAttributeData(OGLConstants::NORMAL.location, normals,
      sizeof(normals), 3, GL_STATIC_DRAW);

  // define tangents
  const GLfloat tangents[] = {
      1.f,  0.f,  0.f,  // front face
      1.f,  0.f,  0.f,
      1.f,  0.f,  0.f,
      1.f,  0.f,  0.f,
     -1.f,  0.f,  0.f,  // rear face ...
     -1.f,  0.f,  0.f,
     -1.f,  0.f,  0.f,
     -1.f,  0.f,  0.f,
      0.f,  0.f,  1.f,
      0.f,  0.f,  1.f,
      0.f,  0.f,  1.f,
      0.f,  0.f,  1.f,
      0.f,  0.f, -1.f,
      0.f,  0.f, -1.f,
      0.f,  0.f, -1.f,
      0.f,  0.f, -1.f,
      1.f,  0.f,  0.f,
      1.f,  0.f,  0.f,
      1.f,  0.f,  0.f,
      1.f,  0.f,  0.f,
     -1.f,  0.f,  0.f,
     -1.f,  0.f,  0.f,
     -1.f,  0.f,  0.f,
     -1.f,  0.f,  0.f
  };
  core->addAttributeData(OGLConstants::TANGENT.location, tangents,
      sizeof(tangents), 3, GL_STATIC_DRAW);

  // define binormals
  const GLfloat binormals[] = {
      0.f,  1.f,  0.f,  // front face
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,  // rear face ...
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,
      0.f,  1.f,  0.f,
      0.f,  0.f, -1.f,
      0.f,  0.f, -1.f,
      0.f,  0.f, -1.f,
      0.f,  0.f, -1.f,
      0.f,  0.f, -1.f,
      0.f,  0.f, -1.f,
      0.f,  0.f, -1.f,
      0.f,  0.f, -1.f
  };
  core->addAttributeData(OGLConstants::BINORMAL.location, binormals,
      sizeof(binormals), 3, GL_STATIC_DRAW);

  // define texture coordinates
  const GLfloat texCoords[] = {
       0.f, 0.f,  // front face
       1.f, 0.f,
       1.f, 1.f,
       0.f, 1.f,
       0.f, 0.f,  // rear face ...
       1.f, 0.f,
       1.f, 1.f,
       0.f, 1.f,
       0.f, 0.f,
       1.f, 0.f,
       1.f, 1.f,
       0.f, 1.f,
       0.f, 0.f,
       1.f, 0.f,
       1.f, 1.f,
       0.f, 1.f,
       0.f, 0.f,
       1.f, 0.f,
       1.f, 1.f,
       0.f, 1.f,
       0.f, 0.f,
       1.f, 0.f,
       1.f, 1.f,
       0.f, 1.f
  };
  core->addAttributeData(OGLConstants::TEX_COORD_0.location, texCoords,
      sizeof(texCoords), 2, GL_STATIC_DRAW);

  // define indices (6 faces * 2 triangles -> 12 triangles)
  const GLuint indices[] = {
       0,  1,  3,  // front face
       1,  2,  3,
       4,  5,  7,  // rear face ...
       5,  6,  7,
       8,  9, 11,
       9, 10, 11,
      12, 13, 15,
      13, 14, 15,
      16, 17, 19,
      17, 18, 19,
      20, 21, 23,
      21, 22, 23
  };
  core->setElementIndexData(indices, sizeof(indices), GL_STATIC_DRAW);

  return core;
}


GeometryCoreSP GeometryCoreFactory::createSphere(GLfloat radius, int nSlices, int nStacks) {

  // create geometry core
  auto core = GeometryCore::create(GL_TRIANGLES, DrawMode::ELEMENTS);

  // define vertices, normals, and texture coordinates
  int nVertices = (nStacks + 1) * (nSlices + 1);
  GLfloat* vertices = new GLfloat[3 * nVertices];
  GLfloat* normals = new GLfloat[3 * nVertices];
  GLfloat* tangents = new GLfloat[3 * nVertices];
  GLfloat* binormals = new GLfloat[3 * nVertices];
  GLfloat* texCoords = new GLfloat[2 * nVertices];

  // stack i starts at i * stackOffset (i = 0, 1, ..., nStacks - 1)
  int stackOffset = nSlices + 1;    // number of vertices per stack edge
  float dPhi = 2.f * PI / nSlices;  // longitude angle step
  float dS = 1.f / nSlices;         // texture coordinate s step
  float dTheta = PI / nStacks;      // latitude angle step
  float dT = 1.f / nStacks;         // texture coordinate t step
  for (int i = 0; i <= nStacks; ++i) {
    GLfloat cosTheta = cos(i * dTheta);
    GLfloat sinTheta = sin(i * dTheta);
    for (int j = 0; j <= nSlices; ++j) {
      GLfloat cosPhi = cos(j * dPhi);
      GLfloat sinPhi = sin(j * dPhi);

      int vertIdx = 3 * (i * stackOffset + j);
      assert(vertIdx + 2 < 3 * nVertices);

      vertices[vertIdx] = radius * cosPhi * sinTheta;       // x
      vertices[vertIdx + 1] = radius * sinPhi * sinTheta;   // y
      vertices[vertIdx + 2] = radius * cosTheta;            // z

      normals[vertIdx] = cosPhi * sinTheta;       // x
      normals[vertIdx + 1] = sinPhi * sinTheta;   // y
      normals[vertIdx + 2] = cosTheta;            // z

      tangents[vertIdx] = -sinPhi;        // x
      tangents[vertIdx + 1] = cosPhi;     // y
      tangents[vertIdx + 2] = 0.f;        // z

      binormals[vertIdx] = -cosPhi * cosTheta;      // x
      binormals[vertIdx + 1] = -sinPhi * cosTheta;  // y
      binormals[vertIdx + 2] = sinTheta;            // z

      int texIdx = 2 * (i * stackOffset + j);
      assert(texIdx + 1 < 2 * nVertices);

      texCoords[texIdx] = j * dS;             // s
      texCoords[texIdx + 1] = 1.f - i * dT;   // t
    }
  }

  core->addAttributeData(OGLConstants::VERTEX.location, vertices,
      3 * nVertices * sizeof(GLfloat), 3, GL_STATIC_DRAW);
  core->addAttributeData(OGLConstants::NORMAL.location, normals,
      3 * nVertices * sizeof(GLfloat), 3, GL_STATIC_DRAW);
  core->addAttributeData(OGLConstants::TANGENT.location, tangents,
      3 * nVertices * sizeof(GLfloat), 3, GL_STATIC_DRAW);
  core->addAttributeData(OGLConstants::BINORMAL.location, binormals,
      3 * nVertices * sizeof(GLfloat), 3, GL_STATIC_DRAW);
  core->addAttributeData(OGLConstants::TEX_COORD_0.location, texCoords,
      2 * nVertices * sizeof(GLfloat), 2, GL_STATIC_DRAW);

  // define indices
  int nTriangles = 2 * nSlices * nStacks;
  GLuint* indices = new GLuint[3 * nTriangles];

  for (int i = 0; i < nStacks; ++i) {
    for (int j = 0; j < nSlices; ++j) {
      int faceIdx = 6 * (i * nSlices + j);
      int vertIdx1 = i * stackOffset + j;
      int vertIdx2 = (i + 1) * stackOffset + j;
      assert(faceIdx + 5 < 3 * nTriangles);
      assert(vertIdx1 + 1 < 3 * nVertices);
      assert(vertIdx2 + 1 < 3 * nVertices);

      indices[faceIdx] = vertIdx1;
      indices[faceIdx + 1] = vertIdx1 + 1;
      indices[faceIdx + 2] = vertIdx2;
      indices[faceIdx + 3] = vertIdx1 + 1;
      indices[faceIdx + 4] = vertIdx2 + 1;
      indices[faceIdx + 5] = vertIdx2;
    }
  }
  core->setElementIndexData(indices, 3 * nTriangles * sizeof(GLuint), GL_STATIC_DRAW);

  delete [] vertices;
  vertices = nullptr;
  delete [] normals;
  normals = nullptr;
  delete [] tangents;
  tangents = nullptr;
  delete [] binormals;
  binormals = nullptr;
  delete [] texCoords;
  texCoords = nullptr;
  delete [] indices;
  indices = nullptr;

  return core;
}


GeometryCoreSP GeometryCoreFactory::createCone(GLfloat radius, GLfloat height,
    int nSlices, int nStacks, bool hasCap) {
  return createConicalFrustum(radius, 0.f, height, nSlices, nStacks, hasCap);
}


GeometryCoreSP GeometryCoreFactory::createCylinder(GLfloat radius, GLfloat height,
    int nSlices, int nStacks, bool hasCaps) {
  return createConicalFrustum(radius, radius, height, nSlices, nStacks, hasCaps);
}


GeometryCoreSP GeometryCoreFactory::createConicalFrustum(GLfloat baseRadius, GLfloat topRadius,
    GLfloat height, int nSlices, int nStacks, bool hasCaps) {

  // check dimensions
  assert(topRadius <= baseRadius + FLT_EPSILON);
  bool hasTopCap = hasCaps && (topRadius > FLT_EPSILON);

  // create geometry core
  auto core = GeometryCore::create(GL_TRIANGLES, DrawMode::ELEMENTS);

  // define vertices, normals, and texture coordinates
  int nVertices = (nSlices + 1) * (nStacks + 1)
      + (hasCaps ? nSlices + 2 : 0)
      + (hasTopCap ? nSlices + 2 : 0);
  GLfloat* vertices = new GLfloat[3 * nVertices];
  GLfloat* normals = new GLfloat[3 * nVertices];
  GLfloat* tangents = new GLfloat[3 * nVertices];
  GLfloat* binormals = new GLfloat[3 * nVertices];
  GLfloat* texCoords = new GLfloat[2 * nVertices];

  // stack i starts at i * stackOffset
  // baseCap starts at (nStacks + 1) * stackOffset
  // topCap starts at (nStacks + 1) * stackOffset + capOffset
  int stackOffset = nSlices + 1;    // number of vertices per stack edge
  int capOffset = nSlices + 2;      // number of vertices per cap
  GLfloat halfHeight = 0.5f * height;
  float dPhi = 2.f * PI / nSlices;  // angle step
  float dS = 1.f / nSlices;         // texture coordinate s step
  GLfloat tanTheta = (baseRadius - topRadius) / height;
  float theta = atan(tanTheta);     // angle of lateral surface
  GLfloat cosTheta = cos(theta);
  GLfloat sinTheta = sin(theta);
  float dZ = height / nStacks;      // z coordinate step
  float dT = 1.f / nStacks;         // texture coordinate t step
  for (int i = 0; i <= nStacks; ++i) {
    GLfloat currRadius = topRadius + (height - i * dZ) * tanTheta;
    for (int j = 0; j <= nSlices; ++j) {
      GLfloat cosPhi = cos(j * dPhi);
      GLfloat sinPhi = sin(j * dPhi);

      int vertIdx = 3 * (i * stackOffset + j);
      assert(vertIdx + 2 < 3 * nVertices);

      vertices[vertIdx] = currRadius * cosPhi;       // x
      vertices[vertIdx + 1] = currRadius * sinPhi;   // y
      vertices[vertIdx + 2] = -halfHeight + i * dZ;  // z

      normals[vertIdx] = cosPhi * cosTheta;       // x
      normals[vertIdx + 1] = sinPhi * cosTheta;   // y
      normals[vertIdx + 2] = sinTheta;            // z

      tangents[vertIdx] = -sinPhi;        // x
      tangents[vertIdx + 1] = cosPhi;     // y
      tangents[vertIdx + 2] = 0.f;        // z

      binormals[vertIdx] = -cosPhi * sinTheta;      // x
      binormals[vertIdx + 1] = -sinPhi * sinTheta;  // y
      binormals[vertIdx + 2] = cosTheta;            // z

      int texIdx = 2 * (i * stackOffset + j);
      assert(texIdx + 1 < 2 * nVertices);

      texCoords[texIdx] = j * dS;       // s
      texCoords[texIdx + 1] = i * dT;   // t
    }
  }

  if (hasCaps) {
    for (int i = 0; i <= nSlices; ++i) {
      GLfloat cosPhi = cos(i * dPhi);
      GLfloat sinPhi = sin(i * dPhi);

      // base cap
      int vertIdx = 3 * ((nStacks + 1) * stackOffset + i);
      assert(vertIdx + 2 < 3 * nVertices);

      vertices[vertIdx] = baseRadius * cosPhi;         // x
      vertices[vertIdx + 1] = baseRadius * sinPhi;     // y
      vertices[vertIdx + 2] = -halfHeight;             // z

      normals[vertIdx] = 0.f;        // x
      normals[vertIdx + 1] = 0.f;    // y
      normals[vertIdx + 2] = -1.f;   // z

      tangents[vertIdx] = 1.f;       // x
      tangents[vertIdx + 1] = 0.f;   // y
      tangents[vertIdx + 2] = 0.f;   // z

      binormals[vertIdx] = 0.f;      // x
      binormals[vertIdx + 1] = -1.f; // y
      binormals[vertIdx + 2] = 0.f;  // z

      int texIdx = 2 * ((nStacks + 1) * stackOffset + i);
      assert(texIdx + 1 < 2 * nVertices);

      texCoords[texIdx] = 0.5f;       // s
      texCoords[texIdx + 1] = 0.5f;   // t

      // top cap
      if (hasTopCap) {
        int vertIdx = 3 * ((nStacks + 1) * stackOffset + capOffset + i);
        assert(vertIdx + 2 < 3 * nVertices);

        vertices[vertIdx] = baseRadius * cosPhi;         // x
        vertices[vertIdx + 1] = baseRadius * sinPhi;     // y
        vertices[vertIdx + 2] = halfHeight;              // z

        normals[vertIdx] = 0.f;        // x
        normals[vertIdx + 1] = 0.f;    // y
        normals[vertIdx + 2] = 1.f;    // z

        tangents[vertIdx] = 1.f;       // x
        tangents[vertIdx + 1] = 0.f;   // y
        tangents[vertIdx + 2] = 0.f;   // z

        binormals[vertIdx] = 0.f;      // x
        binormals[vertIdx + 1] = 1.f;  // y
        binormals[vertIdx + 2] = 0.f;  // z

        int texIdx = 2 * ((nStacks + 1) * stackOffset + capOffset + i);
        assert(texIdx + 1 < 2 * nVertices);

        texCoords[texIdx] = 0.5f;       // s
        texCoords[texIdx + 1] = 0.5f;   // t
      }
    }
  }

  if (hasCaps) {
    // base cap center
    int vertIdx = 3 * ((nStacks + 1) * stackOffset + stackOffset);
    assert(vertIdx + 2 < 3 * nVertices);

    vertices[vertIdx] = 0.f;               // x
    vertices[vertIdx + 1] = 0.f;           // y
    vertices[vertIdx + 2] = -halfHeight;   // z

    normals[vertIdx] = 0.f;        // x
    normals[vertIdx + 1] = 0.f;    // y
    normals[vertIdx + 2] = -1.f;   // z

    tangents[vertIdx] = 1.f;       // x
    tangents[vertIdx + 1] = 0.f;   // y
    tangents[vertIdx + 2] = 0.f;   // z

    binormals[vertIdx] = 0.f;      // x
    binormals[vertIdx + 1] = -1.f; // y
    binormals[vertIdx + 2] = 0.f;  // z

    int texIdx = 2 * ((nStacks + 1) * stackOffset + stackOffset);
    assert(texIdx + 1 < 2 * nVertices);

    texCoords[texIdx] = 0.5f;      // s
    texCoords[texIdx + 1] = 0.5f;  // t

    if (hasTopCap) {
      // top cap center
      int vertIdx = 3 * ((nStacks + 1) * stackOffset + capOffset + stackOffset);
      assert(vertIdx + 2 < 3 * nVertices);

      vertices[vertIdx] = 0.f;               // x
      vertices[vertIdx + 1] = 0.f;           // y
      vertices[vertIdx + 2] = halfHeight;    // z

      normals[vertIdx] = 0.f;        // x
      normals[vertIdx + 1] = 0.f;    // y
      normals[vertIdx + 2] = 1.f;     // z

      tangents[vertIdx] = 1.f;       // x
      tangents[vertIdx + 1] = 0.f;   // y
      tangents[vertIdx + 2] = 0.f;   // z

      binormals[vertIdx] = 0.f;      // x
      binormals[vertIdx + 1] = 1.f;  // y
      binormals[vertIdx + 2] = 0.f;  // z

      int texIdx = 2 * ((nStacks + 1) * stackOffset + capOffset + stackOffset);
      assert(texIdx + 1 < 2 * nVertices);

      texCoords[texIdx] = 0.5f;      // s
      texCoords[texIdx + 1] = 0.5f;  // t
    }
  }

  core->addAttributeData(OGLConstants::VERTEX.location, vertices,
      3 * nVertices * sizeof(GLfloat), 3, GL_STATIC_DRAW);
  core->addAttributeData(OGLConstants::NORMAL.location, normals,
      3 * nVertices * sizeof(GLfloat), 3, GL_STATIC_DRAW);
  core->addAttributeData(OGLConstants::TANGENT.location, tangents,
      3 * nVertices * sizeof(GLfloat), 3, GL_STATIC_DRAW);
  core->addAttributeData(OGLConstants::BINORMAL.location, binormals,
      3 * nVertices * sizeof(GLfloat), 3, GL_STATIC_DRAW);
  core->addAttributeData(OGLConstants::TEX_COORD_0.location, texCoords,
      2 * nVertices * sizeof(GLfloat), 2, GL_STATIC_DRAW);

  // define indices
  int nTriangles = 2 * nSlices * nStacks
      + (hasCaps ? nSlices : 0)
      + (hasTopCap ? nSlices : 0);
  GLuint* indices = new GLuint[3 * nTriangles];

  for (int i = 0; i < nStacks; ++i) {
    for (int j = 0; j < nSlices; ++j) {
      int faceIdx = 6 * (i * nSlices + j);
      int vertIdx1 = i * stackOffset + j;
      int vertIdx2 = (i + 1) * stackOffset + j;
      assert(faceIdx + 5 < 3 * nTriangles);
      assert(vertIdx1 + 1 < 3 * nVertices);
      assert(vertIdx2 + 1 < 3 * nVertices);

      indices[faceIdx] = vertIdx1;
      indices[faceIdx + 1] = vertIdx1 + 1;
      indices[faceIdx + 2] = vertIdx2;
      indices[faceIdx + 3] = vertIdx1 + 1;
      indices[faceIdx + 4] = vertIdx2 + 1;
      indices[faceIdx + 5] = vertIdx2;
    }
  }

  if (hasCaps) {
    for (int i = 0; i < nSlices; ++i) {
      // base cap
      int faceIdx = 3 * (2 * nStacks * nSlices + i);
      int vertIdx1 = (nStacks + 1) * stackOffset + stackOffset;   // center point
      int vertIdx2 = (nStacks + 1) * stackOffset + i;
      assert(faceIdx + 2 < 3 * nTriangles);
      assert(vertIdx1 + 1 < 3 * nVertices);
      assert(vertIdx2 + 1 < 3 * nVertices);

      indices[faceIdx] = vertIdx1;
      indices[faceIdx + 1] = vertIdx2 + 1;
      indices[faceIdx + 2] = vertIdx2;

      if (hasTopCap) {
        // top cap
        int faceIdx = 3 * (2 * nStacks * nSlices + nSlices + i);
        int vertIdx1 = (nStacks + 1) * stackOffset + capOffset + i;
        int vertIdx2 = (nStacks + 1) * stackOffset + capOffset + stackOffset;   // center point
        assert(faceIdx + 2 < 3 * nTriangles);
        assert(vertIdx1 + 1 < 3 * nVertices);
        assert(vertIdx2 + 1 < 3 * nVertices);

        indices[faceIdx] = vertIdx1;
        indices[faceIdx + 1] = vertIdx1 + 1;
        indices[faceIdx + 2] = vertIdx2;
      }
    }
  }
  core->setElementIndexData(indices, 3 * nTriangles * sizeof(GLuint), GL_STATIC_DRAW);

  delete [] vertices;
  vertices = nullptr;
  delete [] normals;
  normals = nullptr;
  delete [] tangents;
  tangents = nullptr;
  delete [] binormals;
  binormals = nullptr;
  delete [] texCoords;
  texCoords = nullptr;
  delete [] indices;
  indices = nullptr;

  return core;
}


GeometryCoreSP GeometryCoreFactory::createTeapot(GLfloat size) {
  // create geometry core
  auto core = GeometryCore::create(GL_TRIANGLES, DrawMode::ELEMENTS);

  // define vertices (800 vertices)
  GLfloat vertices[] = {
      17.83489990234375, 0, 30.573999404907227, 16.452699661254883, -7.000179767608643, 30.573999404907227, 16.223100662231445, -6.902520179748535, 31.51460075378418, 17.586000442504883, 0, 31.51460075378418, 16.48940086364746, -7.015810012817383, 31.828100204467773, 17.87470054626465, 0, 31.828100204467773, 17.031099319458008, -7.246280193328857, 31.51460075378418, 18.46190071105957, 0, 31.51460075378418, 17.62779998779297, -7.500199794769287, 30.573999404907227, 19.108800888061523, 0, 30.573999404907227, 12.662699699401855, -12.662699699401855, 30.573999404907227, 12.486100196838379, -12.486100196838379, 31.51460075378418, 12.690999984741211, -12.690999984741211, 31.828100204467773, 13.10789966583252, -13.10789966583252, 31.51460075378418, 13.56719970703125, -13.56719970703125, 30.573999404907227, 7.000179767608643, -16.452699661254883, 30.573999404907227, 6.902520179748535, -16.223100662231445, 31.51460075378418, 7.015810012817383, -16.48940086364746, 31.828100204467773, 7.246280193328857, -17.031099319458008, 31.51460075378418, 7.500199794769287, -17.62779998779297, 30.573999404907227, 0, -17.83489990234375, 30.573999404907227, 0, -17.586000442504883, 31.51460075378418, 0, -17.87470054626465, 31.828100204467773, 0, -18.46190071105957, 31.51460075378418, 0, -19.108800888061523, 30.573999404907227, 0, -17.83489990234375, 30.573999404907227, -7.483870029449463, -16.452699661254883, 30.573999404907227, -7.106579780578613, -16.223100662231445, 31.51460075378418, 0, -17.586000442504883, 31.51460075378418, -7.07627010345459, -16.48940086364746, 31.828100204467773, 0, -17.87470054626465, 31.828100204467773, -7.25383996963501, -17.031099319458008, 31.51460075378418, 0, -18.46190071105957, 31.51460075378418, -7.500199794769287, -17.62779998779297, 30.573999404907227, 0, -19.108800888061523, 30.573999404907227, -13.092700004577637, -12.662699699401855, 30.573999404907227, -12.667499542236328, -12.486100196838379, 31.51460075378418, -12.744799613952637, -12.690999984741211, 31.828100204467773, -13.11460018157959, -13.10789966583252, 31.51460075378418, -13.56719970703125, -13.56719970703125, 30.573999404907227, -16.61389923095703, -7.000179767608643, 30.573999404907227, -16.291099548339844, -6.902520179748535, 31.51460075378418, -16.50950050354004, -7.015810012817383, 31.828100204467773, -17.033599853515625, -7.246280193328857, 31.51460075378418, -17.62779998779297, -7.500199794769287, 30.573999404907227, -17.83489990234375, 0, 30.573999404907227, -17.586000442504883, 0, 31.51460075378418, -17.87470054626465, 0, 31.828100204467773, -18.46190071105957, 0, 31.51460075378418, -19.108800888061523, 0, 30.573999404907227, -17.83489990234375, 0, 30.573999404907227, -16.452699661254883, 7.000179767608643, 30.573999404907227, -16.223100662231445, 6.902520179748535, 31.51460075378418, -17.586000442504883, 0, 31.51460075378418, -16.48940086364746, 7.015810012817383, 31.828100204467773, -17.87470054626465, 0, 31.828100204467773, -17.031099319458008, 7.246280193328857, 31.51460075378418, -18.46190071105957, 0, 31.51460075378418, -17.62779998779297, 7.500199794769287, 30.573999404907227, -19.108800888061523, 0, 30.573999404907227, -12.662699699401855, 12.662699699401855, 30.573999404907227, -12.486100196838379, 12.486100196838379, 31.51460075378418, -12.690999984741211, 12.690999984741211, 31.828100204467773, -13.10789966583252, 13.10789966583252, 31.51460075378418, -13.56719970703125, 13.56719970703125, 30.573999404907227, -7.000179767608643, 16.452699661254883, 30.573999404907227, -6.902520179748535, 16.223100662231445, 31.51460075378418, -7.015810012817383, 16.48940086364746, 31.828100204467773, -7.246280193328857, 17.031099319458008, 31.51460075378418, -7.500199794769287, 17.62779998779297, 30.573999404907227, 0, 17.83489990234375, 30.573999404907227, 0, 17.586000442504883, 31.51460075378418, 0, 17.87470054626465, 31.828100204467773, 0, 18.46190071105957, 31.51460075378418, 0, 19.108800888061523, 30.573999404907227, 0, 17.83489990234375, 30.573999404907227, 7.000179767608643, 16.452699661254883, 30.573999404907227, 6.902520179748535, 16.223100662231445, 31.51460075378418, 0, 17.586000442504883, 31.51460075378418, 7.015810012817383, 16.48940086364746, 31.828100204467773, 0, 17.87470054626465, 31.828100204467773, 7.246280193328857, 17.031099319458008, 31.51460075378418, 0, 18.46190071105957, 31.51460075378418, 7.500199794769287, 17.62779998779297, 30.573999404907227, 0, 19.108800888061523, 30.573999404907227, 12.662699699401855, 12.662699699401855, 30.573999404907227, 12.486100196838379, 12.486100196838379, 31.51460075378418, 12.690999984741211, 12.690999984741211, 31.828100204467773, 13.10789966583252, 13.10789966583252, 31.51460075378418, 13.56719970703125, 13.56719970703125, 30.573999404907227, 16.452699661254883, 7.000179767608643, 30.573999404907227, 16.223100662231445, 6.902520179748535, 31.51460075378418, 16.48940086364746, 7.015810012817383, 31.828100204467773, 17.031099319458008, 7.246280193328857, 31.51460075378418, 17.62779998779297, 7.500199794769287, 30.573999404907227, 17.83489990234375, 0, 30.573999404907227, 17.586000442504883, 0, 31.51460075378418, 17.87470054626465, 0, 31.828100204467773, 18.46190071105957, 0, 31.51460075378418, 19.108800888061523, 0, 30.573999404907227, 19.108800888061523, 0, 30.573999404907227, 17.62779998779297, -7.500199794769287, 30.573999404907227, 19.785400390625, -8.418190002441406, 25.572900772094727, 21.447599411010742, 0, 25.572900772094727, 21.667600631713867, -9.218990325927734, 20.661399841308594, 23.487899780273438, 0, 20.661399841308594, 22.99880027770996, -9.785409927368164, 15.928999900817871, 24.930999755859375, 0, 15.928999900817871, 23.503799438476562, -10.000300407409668, 11.465299606323242, 25.4783992767334, 0, 11.465299606323242, 13.56719970703125, -13.56719970703125, 30.573999404907227, 15.227800369262695, -15.227800369262695, 25.572900772094727, 16.67639923095703, -16.67639923095703, 20.661399841308594, 17.701000213623047, -17.701000213623047, 15.928999900817871, 18.089599609375, -18.089599609375, 11.465299606323242, 7.500199794769287, -17.62779998779297, 30.573999404907227, 8.418190002441406, -19.785400390625, 25.572900772094727, 9.218990325927734, -21.667600631713867, 20.661399841308594, 9.785409927368164, -22.99880027770996, 15.928999900817871, 10.000300407409668, -23.503799438476562, 11.465299606323242, 0, -19.108800888061523, 30.573999404907227, 0, -21.447599411010742, 25.572900772094727, 0, -23.487899780273438, 20.661399841308594, 0, -24.930999755859375, 15.928999900817871, 0, -25.4783992767334, 11.465299606323242, 0, -19.108800888061523, 30.573999404907227, -7.500199794769287, -17.62779998779297, 30.573999404907227, -8.418190002441406, -19.785400390625, 25.572900772094727, 0, -21.447599411010742, 25.572900772094727, -9.218990325927734, -21.667600631713867, 20.661399841308594, 0, -23.487899780273438, 20.661399841308594, -9.785409927368164, -22.99880027770996, 15.928999900817871, 0, -24.930999755859375, 15.928999900817871, -10.000300407409668, -23.503799438476562, 11.465299606323242, 0, -25.4783992767334, 11.465299606323242, -13.56719970703125, -13.56719970703125, 30.573999404907227, -15.227800369262695, -15.227800369262695, 25.572900772094727, -16.67639923095703, -16.67639923095703, 20.661399841308594, -17.701000213623047, -17.701000213623047, 15.928999900817871, -18.089599609375, -18.089599609375, 11.465299606323242, -17.62779998779297, -7.500199794769287, 30.573999404907227, -19.785400390625, -8.418190002441406, 25.572900772094727, -21.667600631713867, -9.218990325927734, 20.661399841308594, -22.99880027770996, -9.785409927368164, 15.928999900817871, -23.503799438476562, -10.000300407409668, 11.465299606323242, -19.108800888061523, 0, 30.573999404907227, -21.447599411010742, 0, 25.572900772094727, -23.487899780273438, 0, 20.661399841308594, -24.930999755859375, 0, 15.928999900817871, -25.4783992767334, 0, 11.465299606323242, -19.108800888061523, 0, 30.573999404907227, -17.62779998779297, 7.500199794769287, 30.573999404907227, -19.785400390625, 8.418190002441406, 25.572900772094727, -21.447599411010742, 0, 25.572900772094727, -21.667600631713867, 9.218990325927734, 20.661399841308594, -23.487899780273438, 0, 20.661399841308594, -22.99880027770996, 9.785409927368164, 15.928999900817871, -24.930999755859375, 0, 15.928999900817871, -23.503799438476562, 10.000300407409668, 11.465299606323242, -25.4783992767334, 0, 11.465299606323242, -13.56719970703125, 13.56719970703125, 30.573999404907227, -15.227800369262695, 15.227800369262695, 25.572900772094727, -16.67639923095703, 16.67639923095703, 20.661399841308594, -17.701000213623047, 17.701000213623047, 15.928999900817871, -18.089599609375, 18.089599609375, 11.465299606323242, -7.500199794769287, 17.62779998779297, 30.573999404907227, -8.418190002441406, 19.785400390625, 25.572900772094727, -9.218990325927734, 21.667600631713867, 20.661399841308594, -9.785409927368164, 22.99880027770996, 15.928999900817871, -10.000300407409668, 23.503799438476562, 11.465299606323242, 0, 19.108800888061523, 30.573999404907227, 0, 21.447599411010742, 25.572900772094727, 0, 23.487899780273438, 20.661399841308594, 0, 24.930999755859375, 15.928999900817871, 0, 25.4783992767334, 11.465299606323242, 0, 19.108800888061523, 30.573999404907227, 7.500199794769287, 17.62779998779297, 30.573999404907227, 8.418190002441406, 19.785400390625, 25.572900772094727, 0, 21.447599411010742, 25.572900772094727, 9.218990325927734, 21.667600631713867, 20.661399841308594, 0, 23.487899780273438, 20.661399841308594, 9.785409927368164, 22.99880027770996, 15.928999900817871, 0, 24.930999755859375, 15.928999900817871, 10.000300407409668, 23.503799438476562, 11.465299606323242, 0, 25.4783992767334, 11.465299606323242, 13.56719970703125, 13.56719970703125, 30.573999404907227, 15.227800369262695, 15.227800369262695, 25.572900772094727, 16.67639923095703, 16.67639923095703, 20.661399841308594, 17.701000213623047, 17.701000213623047, 15.928999900817871, 18.089599609375, 18.089599609375, 11.465299606323242, 17.62779998779297, 7.500199794769287, 30.573999404907227, 19.785400390625, 8.418190002441406, 25.572900772094727, 21.667600631713867, 9.218990325927734, 20.661399841308594, 22.99880027770996, 9.785409927368164, 15.928999900817871, 23.503799438476562, 10.000300407409668, 11.465299606323242, 19.108800888061523, 0, 30.573999404907227, 21.447599411010742, 0, 25.572900772094727, 23.487899780273438, 0, 20.661399841308594, 24.930999755859375, 0, 15.928999900817871, 25.4783992767334, 0, 11.465299606323242, 25.4783992767334, 0, 11.465299606323242, 23.503799438476562, -10.000300407409668, 11.465299606323242, 22.5856990814209, -9.609620094299316, 7.688300132751465, 24.48310089111328, 0, 7.688300132751465, 20.565799713134766, -8.750229835510254, 4.89661979675293, 22.29360008239746, 0, 4.89661979675293, 18.54599952697754, -7.890830039978027, 3.0006699562072754, 20.104000091552734, 0, 3.0006699562072754, 17.62779998779297, -7.500199794769287, 1.9108799695968628, 19.108800888061523, 0, 1.9108799695968628, 18.089599609375, -18.089599609375, 11.465299606323242, 17.382999420166016, -17.382999420166016, 7.688300132751465, 15.828399658203125, -15.828399658203125, 4.89661979675293, 14.273900032043457, -14.273900032043457, 3.0006699562072754, 13.56719970703125, -13.56719970703125, 1.9108799695968628, 10.000300407409668, -23.503799438476562, 11.465299606323242, 9.609620094299316, -22.5856990814209, 7.688300132751465, 8.750229835510254, -20.565799713134766, 4.89661979675293, 7.890830039978027, -18.54599952697754, 3.0006699562072754, 7.500199794769287, -17.62779998779297, 1.9108799695968628, 0, -25.4783992767334, 11.465299606323242, 0, -24.48310089111328, 7.688300132751465, 0, -22.29360008239746, 4.89661979675293, 0, -20.104000091552734, 3.0006699562072754, 0, -19.108800888061523, 1.9108799695968628, 0, -25.4783992767334, 11.465299606323242, -10.000300407409668, -23.503799438476562, 11.465299606323242, -9.609620094299316, -22.5856990814209, 7.688300132751465, 0, -24.48310089111328, 7.688300132751465, -8.750229835510254, -20.565799713134766, 4.89661979675293, 0, -22.29360008239746, 4.89661979675293, -7.890830039978027, -18.54599952697754, 3.0006699562072754, 0, -20.104000091552734, 3.0006699562072754, -7.500199794769287, -17.62779998779297, 1.9108799695968628, 0, -19.108800888061523, 1.9108799695968628, -18.089599609375, -18.089599609375, 11.465299606323242, -17.382999420166016, -17.382999420166016, 7.688300132751465, -15.828399658203125, -15.828399658203125, 4.89661979675293, -14.273900032043457, -14.273900032043457, 3.0006699562072754, -13.56719970703125, -13.56719970703125, 1.9108799695968628, -23.503799438476562, -10.000300407409668, 11.465299606323242, -22.5856990814209, -9.609620094299316, 7.688300132751465, -20.565799713134766, -8.750229835510254, 4.89661979675293, -18.54599952697754, -7.890830039978027, 3.0006699562072754, -17.62779998779297, -7.500199794769287, 1.9108799695968628, -25.4783992767334, 0, 11.465299606323242, -24.48310089111328, 0, 7.688300132751465, -22.29360008239746, 0, 4.89661979675293, -20.104000091552734, 0, 3.0006699562072754, -19.108800888061523, 0, 1.9108799695968628, -25.4783992767334, 0, 11.465299606323242, -23.503799438476562, 10.000300407409668, 11.465299606323242, -22.5856990814209, 9.609620094299316, 7.688300132751465, -24.48310089111328, 0, 7.688300132751465, -20.565799713134766, 8.750229835510254, 4.89661979675293, -22.29360008239746, 0, 4.89661979675293, -18.54599952697754, 7.890830039978027, 3.0006699562072754, -20.104000091552734, 0, 3.0006699562072754, -17.62779998779297, 7.500199794769287, 1.9108799695968628, -19.108800888061523, 0, 1.9108799695968628, -18.089599609375, 18.089599609375, 11.465299606323242, -17.382999420166016, 17.382999420166016, 7.688300132751465, -15.828399658203125, 15.828399658203125, 4.89661979675293, -14.273900032043457, 14.273900032043457, 3.0006699562072754, -13.56719970703125, 13.56719970703125, 1.9108799695968628, -10.000300407409668, 23.503799438476562, 11.465299606323242, -9.609620094299316, 22.5856990814209, 7.688300132751465, -8.750229835510254, 20.565799713134766, 4.89661979675293, -7.890830039978027, 18.54599952697754, 3.0006699562072754, -7.500199794769287, 17.62779998779297, 1.9108799695968628, 0, 25.4783992767334, 11.465299606323242, 0, 24.48310089111328, 7.688300132751465, 0, 22.29360008239746, 4.89661979675293, 0, 20.104000091552734, 3.0006699562072754, 0, 19.108800888061523, 1.9108799695968628, 0, 25.4783992767334, 11.465299606323242, 10.000300407409668, 23.503799438476562, 11.465299606323242, 9.609620094299316, 22.5856990814209, 7.688300132751465, 0, 24.48310089111328, 7.688300132751465, 8.750229835510254, 20.565799713134766, 4.89661979675293, 0, 22.29360008239746, 4.89661979675293, 7.890830039978027, 18.54599952697754, 3.0006699562072754, 0, 20.104000091552734, 3.0006699562072754, 7.500199794769287, 17.62779998779297, 1.9108799695968628, 0, 19.108800888061523, 1.9108799695968628, 18.089599609375, 18.089599609375, 11.465299606323242, 17.382999420166016, 17.382999420166016, 7.688300132751465, 15.828399658203125, 15.828399658203125, 4.89661979675293, 14.273900032043457, 14.273900032043457, 3.0006699562072754, 13.56719970703125, 13.56719970703125, 1.9108799695968628, 23.503799438476562, 10.000300407409668, 11.465299606323242, 22.5856990814209, 9.609620094299316, 7.688300132751465, 20.565799713134766, 8.750229835510254, 4.89661979675293, 18.54599952697754, 7.890830039978027, 3.0006699562072754, 17.62779998779297, 7.500199794769287, 1.9108799695968628, 25.4783992767334, 0, 11.465299606323242, 24.48310089111328, 0, 7.688300132751465, 22.29360008239746, 0, 4.89661979675293, 20.104000091552734, 0, 3.0006699562072754, 19.108800888061523, 0, 1.9108799695968628, 19.108800888061523, 0, 1.9108799695968628, 17.62779998779297, -7.500199794769287, 1.9108799695968628, 17.228500366210938, -7.330269813537598, 1.2092299461364746, 18.675800323486328, 0, 1.2092299461364746, 15.093799591064453, -6.422039985656738, 0.5971490144729614, 16.361900329589844, 0, 0.5971490144729614, 9.819259643554688, -4.177840232849121, 0.16421599686145782, 10.644200325012207, 0, 0.16421599686145782, 0, 0, 0, 0, 0, 0, 13.56719970703125, -13.56719970703125, 1.9108799695968628, 13.25979995727539, -13.25979995727539, 1.2092299461364746, 11.616900444030762, -11.616900444030762, 0.5971490144729614, 7.557370185852051, -7.557370185852051, 0.16421599686145782, 0, 0, 0, 7.500199794769287, -17.62779998779297, 1.9108799695968628, 7.330269813537598, -17.228500366210938, 1.2092299461364746, 6.422039985656738, -15.093799591064453, 0.5971490144729614, 4.177840232849121, -9.819259643554688, 0.16421599686145782, 0, 0, 0, 0, -19.108800888061523, 1.9108799695968628, 0, -18.675800323486328, 1.2092299461364746, 0, -16.361900329589844, 0.5971490144729614, 0, -10.644200325012207, 0.16421599686145782, 0, 0, 0, 0, -19.108800888061523, 1.9108799695968628, -7.500199794769287, -17.62779998779297, 1.9108799695968628, -7.330269813537598, -17.228500366210938, 1.2092299461364746, 0, -18.675800323486328, 1.2092299461364746, -6.422039985656738, -15.093799591064453, 0.5971490144729614, 0, -16.361900329589844, 0.5971490144729614, -4.177840232849121, -9.819259643554688, 0.16421599686145782, 0, -10.644200325012207, 0.16421599686145782, 0, 0, 0, 0, 0, 0, -13.56719970703125, -13.56719970703125, 1.9108799695968628, -13.25979995727539, -13.25979995727539, 1.2092299461364746, -11.616900444030762, -11.616900444030762, 0.5971490144729614, -7.557370185852051, -7.557370185852051, 0.16421599686145782, 0, 0, 0, -17.62779998779297, -7.500199794769287, 1.9108799695968628, -17.228500366210938, -7.330269813537598, 1.2092299461364746, -15.093799591064453, -6.422039985656738, 0.5971490144729614, -9.819259643554688, -4.177840232849121, 0.16421599686145782, 0, 0, 0, -19.108800888061523, 0, 1.9108799695968628, -18.675800323486328, 0, 1.2092299461364746, -16.361900329589844, 0, 0.5971490144729614, -10.644200325012207, 0, 0.16421599686145782, 0, 0, 0, -19.108800888061523, 0, 1.9108799695968628, -17.62779998779297, 7.500199794769287, 1.9108799695968628, -17.228500366210938, 7.330269813537598, 1.2092299461364746, -18.675800323486328, 0, 1.2092299461364746, -15.093799591064453, 6.422039985656738, 0.5971490144729614, -16.361900329589844, 0, 0.5971490144729614, -9.819259643554688, 4.177840232849121, 0.16421599686145782, -10.644200325012207, 0, 0.16421599686145782, 0, 0, 0, 0, 0, 0, -13.56719970703125, 13.56719970703125, 1.9108799695968628, -13.25979995727539, 13.25979995727539, 1.2092299461364746, -11.616900444030762, 11.616900444030762, 0.5971490144729614, -7.557370185852051, 7.557370185852051, 0.16421599686145782, 0, 0, 0, -7.500199794769287, 17.62779998779297, 1.9108799695968628, -7.330269813537598, 17.228500366210938, 1.2092299461364746, -6.422039985656738, 15.093799591064453, 0.5971490144729614, -4.177840232849121, 9.819259643554688, 0.16421599686145782, 0, 0, 0, 0, 19.108800888061523, 1.9108799695968628, 0, 18.675800323486328, 1.2092299461364746, 0, 16.361900329589844, 0.5971490144729614, 0, 10.644200325012207, 0.16421599686145782, 0, 0, 0, 0, 19.108800888061523, 1.9108799695968628, 7.500199794769287, 17.62779998779297, 1.9108799695968628, 7.330269813537598, 17.228500366210938, 1.2092299461364746, 0, 18.675800323486328, 1.2092299461364746, 6.422039985656738, 15.093799591064453, 0.5971490144729614, 0, 16.361900329589844, 0.5971490144729614, 4.177840232849121, 9.819259643554688, 0.16421599686145782, 0, 10.644200325012207, 0.16421599686145782, 0, 0, 0, 0, 0, 0, 13.56719970703125, 13.56719970703125, 1.9108799695968628, 13.25979995727539, 13.25979995727539, 1.2092299461364746, 11.616900444030762, 11.616900444030762, 0.5971490144729614, 7.557370185852051, 7.557370185852051, 0.16421599686145782, 0, 0, 0, 17.62779998779297, 7.500199794769287, 1.9108799695968628, 17.228500366210938, 7.330269813537598, 1.2092299461364746, 15.093799591064453, 6.422039985656738, 0.5971490144729614, 9.819259643554688, 4.177840232849121, 0.16421599686145782, 0, 0, 0, 19.108800888061523, 0, 1.9108799695968628, 18.675800323486328, 0, 1.2092299461364746, 16.361900329589844, 0, 0.5971490144729614, 10.644200325012207, 0, 0.16421599686145782, 0, 0, 0, -20.382699966430664, 0, 25.796899795532227, -20.1835994720459, -2.149739980697632, 26.244699478149414, -26.511600494384766, -2.149739980697632, 26.192899703979492, -26.334299087524414, 0, 25.752099990844727, -31.156299591064453, -2.149739980697632, 25.830400466918945, -30.733299255371094, 0, 25.438600540161133, -34.016998291015625, -2.149739980697632, 24.846500396728516, -33.46030044555664, 0, 24.587600708007812, -34.99290084838867, -2.149739980697632, 22.930500030517578, -34.39580154418945, 0, 22.930500030517578, -19.74570083618164, -2.8663198947906494, 27.229999542236328, -26.901599884033203, -2.8663198947906494, 27.162799835205078, -32.08679962158203, -2.8663198947906494, 26.69260025024414, -35.241798400878906, -2.8663198947906494, 25.416200637817383, -36.30670166015625, -2.8663198947906494, 22.930500030517578, -19.30780029296875, -2.149739980697632, 28.215299606323242, -27.29159927368164, -2.149739980697632, 28.132699966430664, -33.017398834228516, -2.149739980697632, 27.55470085144043, -36.46649932861328, -2.149739980697632, 25.98579978942871, -37.620399475097656, -2.149739980697632, 22.930500030517578, -19.108800888061523, 0, 28.66320037841797, -27.468900680541992, 0, 28.57360076904297, -33.440399169921875, 0, 27.94659996032715, -37.02330017089844, 0, 26.244699478149414, -38.21760177612305, 0, 22.930500030517578, -19.108800888061523, 0, 28.66320037841797, -19.30780029296875, 2.149739980697632, 28.215299606323242, -27.29159927368164, 2.149739980697632, 28.132699966430664, -27.468900680541992, 0, 28.57360076904297, -33.017398834228516, 2.149739980697632, 27.55470085144043, -33.440399169921875, 0, 27.94659996032715, -36.46649932861328, 2.149739980697632, 25.98579978942871, -37.02330017089844, 0, 26.244699478149414, -37.620399475097656, 2.149739980697632, 22.930500030517578, -38.21760177612305, 0, 22.930500030517578, -19.74570083618164, 2.8663198947906494, 27.229999542236328, -26.901599884033203, 2.8663198947906494, 27.162799835205078, -32.08679962158203, 2.8663198947906494, 26.69260025024414, -35.241798400878906, 2.8663198947906494, 25.416200637817383, -36.30670166015625, 2.8663198947906494, 22.930500030517578, -20.1835994720459, 2.149739980697632, 26.244699478149414, -26.511600494384766, 2.149739980697632, 26.192899703979492, -31.156299591064453, 2.149739980697632, 25.830400466918945, -34.016998291015625, 2.149739980697632, 24.846500396728516, -34.99290084838867, 2.149739980697632, 22.930500030517578, -20.382699966430664, 0, 25.796899795532227, -26.334299087524414, 0, 25.752099990844727, -30.733299255371094, 0, 25.438600540161133, -33.46030044555664, 0, 24.587600708007812, -34.39580154418945, 0, 22.930500030517578, -34.39580154418945, 0, 22.930500030517578, -34.99290084838867, -2.149739980697632, 22.930500030517578, -34.44089889526367, -2.149739980697632, 20.082199096679688, -33.89820098876953, 0, 20.33289909362793, -32.711299896240234, -2.149739980697632, 16.81529998779297, -32.32569885253906, 0, 17.197900772094727, -29.69420051574707, -2.149739980697632, 13.590499877929688, -29.558900833129883, 0, 14.062899589538574, -25.279300689697266, -2.149739980697632, 10.8681001663208, -25.4783992767334, 0, 11.465299606323242, -36.30670166015625, -2.8663198947906494, 22.930500030517578, -35.6348991394043, -2.8663198947906494, 19.530500411987305, -33.55979919433594, -2.8663198947906494, 15.973699569702148, -29.99180030822754, -2.8663198947906494, 12.551300048828125, -24.841400146484375, -2.8663198947906494, 9.554389953613281, -37.620399475097656, -2.149739980697632, 22.930500030517578, -36.82889938354492, -2.149739980697632, 18.97879981994629, -34.408199310302734, -2.149739980697632, 15.132100105285645, -30.289499282836914, -2.149739980697632, 11.512200355529785, -24.403499603271484, -2.149739980697632, 8.240659713745117, -38.21760177612305, 0, 22.930500030517578, -37.37160110473633, 0, 18.728099822998047, -34.79389953613281, 0, 14.749600410461426, -30.424800872802734, 0, 11.039799690246582, -24.204500198364258, 0, 7.643509864807129, -38.21760177612305, 0, 22.930500030517578, -37.620399475097656, 2.149739980697632, 22.930500030517578, -36.82889938354492, 2.149739980697632, 18.97879981994629, -37.37160110473633, 0, 18.728099822998047, -34.408199310302734, 2.149739980697632, 15.132100105285645, -34.79389953613281, 0, 14.749600410461426, -30.289499282836914, 2.149739980697632, 11.512200355529785, -30.424800872802734, 0, 11.039799690246582, -24.403499603271484, 2.149739980697632, 8.240659713745117, -24.204500198364258, 0, 7.643509864807129, -36.30670166015625, 2.8663198947906494, 22.930500030517578, -35.6348991394043, 2.8663198947906494, 19.530500411987305, -33.55979919433594, 2.8663198947906494, 15.973699569702148, -29.99180030822754, 2.8663198947906494, 12.551300048828125, -24.841400146484375, 2.8663198947906494, 9.554389953613281, -34.99290084838867, 2.149739980697632, 22.930500030517578, -34.44089889526367, 2.149739980697632, 20.082199096679688, -32.711299896240234, 2.149739980697632, 16.81529998779297, -29.69420051574707, 2.149739980697632, 13.590499877929688, -25.279300689697266, 2.149739980697632, 10.8681001663208, -34.39580154418945, 0, 22.930500030517578, -33.89820098876953, 0, 20.33289909362793, -32.32569885253906, 0, 17.197900772094727, -29.558900833129883, 0, 14.062899589538574, -25.4783992767334, 0, 11.465299606323242, 21.656600952148438, 0, 18.15329933166504, 21.656600952148438, -4.729420185089111, 16.511199951171875, 28.233999252319336, -4.270359992980957, 18.339000701904297, 27.76740074157715, 0, 19.55660057067871, 31.011899948120117, -3.2604401111602783, 22.221399307250977, 30.4148006439209, 0, 22.930500030517578, 32.59560012817383, -2.2505099773406982, 26.764400482177734, 31.867900848388672, 0, 27.020999908447266, 35.5900993347168, -1.791450023651123, 30.573999404907227, 34.39580154418945, 0, 30.573999404907227, 21.656600952148438, -6.3059000968933105, 12.89840030670166, 29.260299682617188, -5.693819999694824, 15.660200119018555, 32.32569885253906, -4.347249984741211, 20.661399841308594, 34.19670104980469, -3.0006699562072754, 26.199899673461914, 38.21760177612305, -2.3886001110076904, 30.573999404907227, 21.656600952148438, -4.729420185089111, 9.285670280456543, 30.286699295043945, -4.270359992980957, 12.981499671936035, 33.639400482177734, -3.2604401111602783, 19.101299285888672, 35.79790115356445, -2.2505099773406982, 25.635400772094727, 40.845001220703125, -1.791450023651123, 30.573999404907227, 21.656600952148438, 0, 7.643509864807129, 30.75320053100586, 0, 11.763799667358398, 34.23659896850586, 0, 18.392200469970703, 36.52560043334961, 0, 25.378799438476562, 42.03929901123047, 0, 30.573999404907227, 21.656600952148438, 0, 7.643509864807129, 21.656600952148438, 4.729420185089111, 9.285670280456543, 30.286699295043945, 4.270359992980957, 12.981499671936035, 30.75320053100586, 0, 11.763799667358398, 33.639400482177734, 3.2604401111602783, 19.101299285888672, 34.23659896850586, 0, 18.392200469970703, 35.79790115356445, 2.2505099773406982, 25.635400772094727, 36.52560043334961, 0, 25.378799438476562, 40.845001220703125, 1.791450023651123, 30.573999404907227, 42.03929901123047, 0, 30.573999404907227, 21.656600952148438, 6.3059000968933105, 12.89840030670166, 29.260299682617188, 5.693819999694824, 15.660200119018555, 32.32569885253906, 4.347249984741211, 20.661399841308594, 34.19670104980469, 3.0006699562072754, 26.199899673461914, 38.21760177612305, 2.3886001110076904, 30.573999404907227, 21.656600952148438, 4.729420185089111, 16.511199951171875, 28.233999252319336, 4.270359992980957, 18.339000701904297, 31.011899948120117, 3.2604401111602783, 22.221399307250977, 32.59560012817383, 2.2505099773406982, 26.764400482177734, 35.5900993347168, 1.791450023651123, 30.573999404907227, 21.656600952148438, 0, 18.15329933166504, 27.76740074157715, 0, 19.55660057067871, 30.4148006439209, 0, 22.930500030517578, 31.867900848388672, 0, 27.020999908447266, 34.39580154418945, 0, 30.573999404907227, 34.39580154418945, 0, 30.573999404907227, 35.5900993347168, -1.791450023651123, 30.573999404907227, 36.59049987792969, -1.679479956626892, 31.137699127197266, 35.3114013671875, 0, 31.111499786376953, 37.18870162963867, -1.4331599473953247, 31.332599639892578, 35.98820114135742, 0, 31.290599822998047, 37.206600189208984, -1.1868300437927246, 31.1481990814209, 36.187198638916016, 0, 31.111499786376953, 36.46590042114258, -1.074869990348816, 30.573999404907227, 35.669700622558594, 0, 30.573999404907227, 38.21760177612305, -2.3886001110076904, 30.573999404907227, 39.40439987182617, -2.2393100261688232, 31.195499420166016, 39.829898834228516, -1.9108799695968628, 31.424999237060547, 39.44919967651367, -1.582450032234192, 31.229000091552734, 38.21760177612305, -1.4331599473953247, 30.573999404907227, 40.845001220703125, -1.791450023651123, 30.573999404907227, 42.218299865722656, -1.679479956626892, 31.25320053100586, 42.47100067138672, -1.4331599473953247, 31.51740074157715, 41.69169998168945, -1.1868300437927246, 31.309900283813477, 39.969200134277344, -1.074869990348816, 30.573999404907227, 42.03929901123047, 0, 30.573999404907227, 43.49729919433594, 0, 31.279399871826172, 43.67150115966797, 0, 31.55929946899414, 42.71110153198242, 0, 31.346599578857422, 40.76539993286133, 0, 30.573999404907227, 42.03929901123047, 0, 30.573999404907227, 40.845001220703125, 1.791450023651123, 30.573999404907227, 42.218299865722656, 1.679479956626892, 31.25320053100586, 43.49729919433594, 0, 31.279399871826172, 42.47100067138672, 1.4331599473953247, 31.51740074157715, 43.67150115966797, 0, 31.55929946899414, 41.69169998168945, 1.1868300437927246, 31.309900283813477, 42.71110153198242, 0, 31.346599578857422, 39.969200134277344, 1.074869990348816, 30.573999404907227, 40.76539993286133, 0, 30.573999404907227, 38.21760177612305, 2.3886001110076904, 30.573999404907227, 39.40439987182617, 2.2393100261688232, 31.195499420166016, 39.829898834228516, 1.9108799695968628, 31.424999237060547, 39.44919967651367, 1.582450032234192, 31.229000091552734, 38.21760177612305, 1.4331599473953247, 30.573999404907227, 35.5900993347168, 1.791450023651123, 30.573999404907227, 36.59049987792969, 1.679479956626892, 31.137699127197266, 37.18870162963867, 1.4331599473953247, 31.332599639892578, 37.206600189208984, 1.1868300437927246, 31.1481990814209, 36.46590042114258, 1.074869990348816, 30.573999404907227, 34.39580154418945, 0, 30.573999404907227, 35.3114013671875, 0, 31.111499786376953, 35.98820114135742, 0, 31.290599822998047, 36.187198638916016, 0, 31.111499786376953, 35.669700622558594, 0, 30.573999404907227, 0, 0, 40.12839889526367, 0, 0, 40.12839889526367, 4.004499912261963, -1.7077000141143799, 39.501399993896484, 4.339280128479004, 0, 39.501399993896484, 3.8207099437713623, -1.6290700435638428, 37.97869873046875, 4.140230178833008, 0, 37.97869873046875, 2.314160108566284, -0.985912024974823, 36.09769821166992, 2.5080299377441406, 0, 36.09769821166992, 2.3503799438476562, -1.0000300407409668, 34.39580154418945, 2.547840118408203, 0, 34.39580154418945, 0, 0, 40.12839889526367, 3.0849199295043945, -3.0849199295043945, 39.501399993896484, 2.943150043487549, -2.943150043487549, 37.97869873046875, 1.782039999961853, -1.782039999961853, 36.09769821166992, 1.8089599609375, -1.8089599609375, 34.39580154418945, 0, 0, 40.12839889526367, 1.7077000141143799, -4.004499912261963, 39.501399993896484, 1.6290700435638428, -3.8207099437713623, 37.97869873046875, 0.985912024974823, -2.314160108566284, 36.09769821166992, 1.0000300407409668, -2.3503799438476562, 34.39580154418945, 0, 0, 40.12839889526367, 0, -4.339280128479004, 39.501399993896484, 0, -4.140230178833008, 37.97869873046875, 0, -2.5080299377441406, 36.09769821166992, 0, -2.547840118408203, 34.39580154418945, 0, 0, 40.12839889526367, 0, 0, 40.12839889526367, -1.7077000141143799, -4.004499912261963, 39.501399993896484, 0, -4.339280128479004, 39.501399993896484, -1.6290700435638428, -3.8207099437713623, 37.97869873046875, 0, -4.140230178833008, 37.97869873046875, -0.985912024974823, -2.314160108566284, 36.09769821166992, 0, -2.5080299377441406, 36.09769821166992, -1.0000300407409668, -2.3503799438476562, 34.39580154418945, 0, -2.547840118408203, 34.39580154418945, 0, 0, 40.12839889526367, -3.0849199295043945, -3.0849199295043945, 39.501399993896484, -2.943150043487549, -2.943150043487549, 37.97869873046875, -1.782039999961853, -1.782039999961853, 36.09769821166992, -1.8089599609375, -1.8089599609375, 34.39580154418945, 0, 0, 40.12839889526367, -4.004499912261963, -1.7077000141143799, 39.501399993896484, -3.8207099437713623, -1.6290700435638428, 37.97869873046875, -2.314160108566284, -0.985912024974823, 36.09769821166992, -2.3503799438476562, -1.0000300407409668, 34.39580154418945, 0, 0, 40.12839889526367, -4.339280128479004, 0, 39.501399993896484, -4.140230178833008, 0, 37.97869873046875, -2.5080299377441406, 0, 36.09769821166992, -2.547840118408203, 0, 34.39580154418945, 0, 0, 40.12839889526367, 0, 0, 40.12839889526367, -4.004499912261963, 1.7077000141143799, 39.501399993896484, -4.339280128479004, 0, 39.501399993896484, -3.8207099437713623, 1.6290700435638428, 37.97869873046875, -4.140230178833008, 0, 37.97869873046875, -2.314160108566284, 0.985912024974823, 36.09769821166992, -2.5080299377441406, 0, 36.09769821166992, -2.3503799438476562, 1.0000300407409668, 34.39580154418945, -2.547840118408203, 0, 34.39580154418945, 0, 0, 40.12839889526367, -3.0849199295043945, 3.0849199295043945, 39.501399993896484, -2.943150043487549, 2.943150043487549, 37.97869873046875, -1.782039999961853, 1.782039999961853, 36.09769821166992, -1.8089599609375, 1.8089599609375, 34.39580154418945, 0, 0, 40.12839889526367, -1.7077000141143799, 4.004499912261963, 39.501399993896484, -1.6290700435638428, 3.8207099437713623, 37.97869873046875, -0.985912024974823, 2.314160108566284, 36.09769821166992, -1.0000300407409668, 2.3503799438476562, 34.39580154418945, 0, 0, 40.12839889526367, 0, 4.339280128479004, 39.501399993896484, 0, 4.140230178833008, 37.97869873046875, 0, 2.5080299377441406, 36.09769821166992, 0, 2.547840118408203, 34.39580154418945, 0, 0, 40.12839889526367, 0, 0, 40.12839889526367, 1.7077000141143799, 4.004499912261963, 39.501399993896484, 0, 4.339280128479004, 39.501399993896484, 1.6290700435638428, 3.8207099437713623, 37.97869873046875, 0, 4.140230178833008, 37.97869873046875, 0.985912024974823, 2.314160108566284, 36.09769821166992, 0, 2.5080299377441406, 36.09769821166992, 1.0000300407409668, 2.3503799438476562, 34.39580154418945, 0, 2.547840118408203, 34.39580154418945, 0, 0, 40.12839889526367, 3.0849199295043945, 3.0849199295043945, 39.501399993896484, 2.943150043487549, 2.943150043487549, 37.97869873046875, 1.782039999961853, 1.782039999961853, 36.09769821166992, 1.8089599609375, 1.8089599609375, 34.39580154418945, 0, 0, 40.12839889526367, 4.004499912261963, 1.7077000141143799, 39.501399993896484, 3.8207099437713623, 1.6290700435638428, 37.97869873046875, 2.314160108566284, 0.985912024974823, 36.09769821166992, 2.3503799438476562, 1.0000300407409668, 34.39580154418945, 0, 0, 40.12839889526367, 4.339280128479004, 0, 39.501399993896484, 4.140230178833008, 0, 37.97869873046875, 2.5080299377441406, 0, 36.09769821166992, 2.547840118408203, 0, 34.39580154418945, 2.547840118408203, 0, 34.39580154418945, 2.3503799438476562, -1.0000300407409668, 34.39580154418945, 5.361800193786621, -2.2813100814819336, 33.261199951171875, 5.812250137329102, 0, 33.261199951171875, 9.695320129394531, -4.125110149383545, 32.484901428222656, 10.50979995727539, 0, 32.484901428222656, 13.58810043334961, -5.781400203704834, 31.708599090576172, 14.729700088500977, 0, 31.708599090576172, 15.27750015258789, -6.5001702308654785, 30.573999404907227, 16.56089973449707, 0, 30.573999404907227, 1.8089599609375, -1.8089599609375, 34.39580154418945, 4.126699924468994, -4.126699924468994, 33.261199951171875, 7.461979866027832, -7.461979866027832, 32.484901428222656, 10.458100318908691, -10.458100318908691, 31.708599090576172, 11.758299827575684, -11.758299827575684, 30.573999404907227, 1.0000300407409668, -2.3503799438476562, 34.39580154418945, 2.2813100814819336, -5.361800193786621, 33.261199951171875, 4.125110149383545, -9.695320129394531, 32.484901428222656, 5.781400203704834, -13.58810043334961, 31.708599090576172, 6.5001702308654785, -15.27750015258789, 30.573999404907227, 0, -2.547840118408203, 34.39580154418945, 0, -5.812250137329102, 33.261199951171875, 0, -10.50979995727539, 32.484901428222656, 0, -14.729700088500977, 31.708599090576172, 0, -16.56089973449707, 30.573999404907227, 0, -2.547840118408203, 34.39580154418945, -1.0000300407409668, -2.3503799438476562, 34.39580154418945, -2.2813100814819336, -5.361800193786621, 33.261199951171875, 0, -5.812250137329102, 33.261199951171875, -4.125110149383545, -9.695320129394531, 32.484901428222656, 0, -10.50979995727539, 32.484901428222656, -5.781400203704834, -13.58810043334961, 31.708599090576172, 0, -14.729700088500977, 31.708599090576172, -6.5001702308654785, -15.27750015258789, 30.573999404907227, 0, -16.56089973449707, 30.573999404907227, -1.8089599609375, -1.8089599609375, 34.39580154418945, -4.126699924468994, -4.126699924468994, 33.261199951171875, -7.461979866027832, -7.461979866027832, 32.484901428222656, -10.458100318908691, -10.458100318908691, 31.708599090576172, -11.758299827575684, -11.758299827575684, 30.573999404907227, -2.3503799438476562, -1.0000300407409668, 34.39580154418945, -5.361800193786621, -2.2813100814819336, 33.261199951171875, -9.695320129394531, -4.125110149383545, 32.484901428222656, -13.58810043334961, -5.781400203704834, 31.708599090576172, -15.27750015258789, -6.5001702308654785, 30.573999404907227, -2.547840118408203, 0, 34.39580154418945, -5.812250137329102, 0, 33.261199951171875, -10.50979995727539, 0, 32.484901428222656, -14.729700088500977, 0, 31.708599090576172, -16.56089973449707, 0, 30.573999404907227, -2.547840118408203, 0, 34.39580154418945, -2.3503799438476562, 1.0000300407409668, 34.39580154418945, -5.361800193786621, 2.2813100814819336, 33.261199951171875, -5.812250137329102, 0, 33.261199951171875, -9.695320129394531, 4.125110149383545, 32.484901428222656, -10.50979995727539, 0, 32.484901428222656, -13.58810043334961, 5.781400203704834, 31.708599090576172, -14.729700088500977, 0, 31.708599090576172, -15.27750015258789, 6.5001702308654785, 30.573999404907227, -16.56089973449707, 0, 30.573999404907227, -1.8089599609375, 1.8089599609375, 34.39580154418945, -4.126699924468994, 4.126699924468994, 33.261199951171875, -7.461979866027832, 7.461979866027832, 32.484901428222656, -10.458100318908691, 10.458100318908691, 31.708599090576172, -11.758299827575684, 11.758299827575684, 30.573999404907227, -1.0000300407409668, 2.3503799438476562, 34.39580154418945, -2.2813100814819336, 5.361800193786621, 33.261199951171875, -4.125110149383545, 9.695320129394531, 32.484901428222656, -5.781400203704834, 13.58810043334961, 31.708599090576172, -6.5001702308654785, 15.27750015258789, 30.573999404907227, 0, 2.547840118408203, 34.39580154418945, 0, 5.812250137329102, 33.261199951171875, 0, 10.50979995727539, 32.484901428222656, 0, 14.729700088500977, 31.708599090576172, 0, 16.56089973449707, 30.573999404907227, 0, 2.547840118408203, 34.39580154418945, 1.0000300407409668, 2.3503799438476562, 34.39580154418945, 2.2813100814819336, 5.361800193786621, 33.261199951171875, 0, 5.812250137329102, 33.261199951171875, 4.125110149383545, 9.695320129394531, 32.484901428222656, 0, 10.50979995727539, 32.484901428222656, 5.781400203704834, 13.58810043334961, 31.708599090576172, 0, 14.729700088500977, 31.708599090576172, 6.5001702308654785, 15.27750015258789, 30.573999404907227, 0, 16.56089973449707, 30.573999404907227, 1.8089599609375, 1.8089599609375, 34.39580154418945, 4.126699924468994, 4.126699924468994, 33.261199951171875, 7.461979866027832, 7.461979866027832, 32.484901428222656, 10.458100318908691, 10.458100318908691, 31.708599090576172, 11.758299827575684, 11.758299827575684, 30.573999404907227, 2.3503799438476562, 1.0000300407409668, 34.39580154418945, 5.361800193786621, 2.2813100814819336, 33.261199951171875, 9.695320129394531, 4.125110149383545, 32.484901428222656, 13.58810043334961, 5.781400203704834, 31.708599090576172, 15.27750015258789, 6.5001702308654785, 30.573999404907227, 2.547840118408203, 0, 34.39580154418945, 5.812250137329102, 0, 33.261199951171875, 10.50979995727539, 0, 32.484901428222656, 14.729700088500977, 0, 31.708599090576172, 16.56089973449707, 0, 30.573999404907227
  };
  // scale and translate vertices to unit size and origin center
  GLfloat factor = size / 50.f;
  int nVertices = sizeof(vertices) / sizeof(GLfloat);
  for (int i = 0; i < nVertices; ++i) {
    vertices[i] *= factor;
  }
  for (int i = 2; i < nVertices; i += 3) {
    vertices[i] -= 0.4f;
  }
  core->addAttributeData(OGLConstants::VERTEX.location, vertices,
      sizeof(vertices), 3, GL_STATIC_DRAW);

  // define normals
  const GLfloat normals[] = {
      -0.9667419791221619, 0, -0.25575199723243713, -0.8930140137672424, 0.3698819875717163, -0.2563450038433075, -0.8934370279312134, 0.36910200119018555, 0.2559970021247864, -0.9668239951133728, 0, 0.2554430067539215, -0.0838799998164177, 0.03550700098276138, 0.9958429932594299, -0.09205400198698044, 0, 0.9957540035247803, 0.629721999168396, -0.2604379951953888, 0.7318620085716248, 0.6820489764213562, 0, 0.7313070297241211, 0.803725004196167, -0.3325839936733246, 0.4933690130710602, 0.8703010082244873, 0, 0.4925200045108795, -0.6834070086479187, 0.6834070086479187, -0.2567310035228729, -0.6835309863090515, 0.6835309863090515, 0.25606799125671387, -0.06492599844932556, 0.06492500007152557, 0.9957759976387024, 0.48139700293540955, -0.48139700293540955, 0.7324709892272949, 0.6148040294647217, -0.6148040294647217, 0.4939970076084137, -0.3698819875717163, 0.8930140137672424, -0.2563450038433075, -0.36910200119018555, 0.8934370279312134, 0.2559959888458252, -0.03550700098276138, 0.0838790014386177, 0.9958429932594299, 0.26043900847435, -0.6297230124473572, 0.7318609952926636, 0.3325839936733246, -0.803725004196167, 0.4933690130710602, -0.002848000032827258, 0.9661769866943359, -0.25786298513412476, -0.001921999966725707, 0.9670090079307556, 0.2547360062599182, -0.00026500000967644155, 0.09227199852466583, 0.9957339763641357, 0.00002300000051036477, -0.6820600032806396, 0.7312960028648376, 0, -0.8703010082244873, 0.4925200045108795, -0.002848000032827258, 0.9661769866943359, -0.25786298513412476, 0.37905800342559814, 0.852770984172821, -0.35929998755455017, 0.37711000442504883, 0.9140909910202026, 0.14908500015735626, -0.001921999966725707, 0.9670090079307556, 0.2547360062599182, 0.0275030005723238, 0.12255500257015228, 0.9920809864997864, -0.00026500000967644155, 0.09227199852466583, 0.9957339763641357, -0.26100900769233704, -0.6353650093078613, 0.7267630100250244, 0.00002300000051036477, -0.6820600032806396, 0.7312960028648376, -0.33248499035835266, -0.8042709827423096, 0.4925459921360016, 0, -0.8703010082244873, 0.4925200045108795, 0.6635469794273376, 0.6252639889717102, -0.4107919931411743, 0.712664008140564, 0.6976209878921509, 0.07372400164604187, 0.09972699731588364, 0.12198299914598465, 0.98750901222229, -0.4873189926147461, -0.4885669946670532, 0.7237560153007507, -0.6152420043945312, -0.6154839992523193, 0.4926010072231293, 0.8800280094146729, 0.3387089967727661, -0.3329069912433624, 0.9172769784927368, 0.36149299144744873, 0.16711199283599854, 0.11358699947595596, 0.04806999862194061, 0.9923650026321411, -0.6341490149497986, -0.2618879973888397, 0.7275090217590332, -0.8041260242462158, -0.33270499110221863, 0.49263399839401245, 0.9666900038719177, -0.010453999973833561, -0.2557379901409149, 0.967441976070404, -0.00810300000011921, 0.25296199321746826, 0.0934389978647232, -0.0012799999676644802, 0.9956240057945251, -0.6821659803390503, 0.0003429999924264848, 0.7311969995498657, -0.8703219890594482, 0.00005400000009103678, 0.492482990026474, 0.9666900038719177, -0.010453999973833561, -0.2557379901409149, 0.8930140137672424, -0.3698819875717163, -0.2563450038433075, 0.8934370279312134, -0.36910200119018555, 0.2559970021247864, 0.967441976070404, -0.00810300000011921, 0.25296199321746826, 0.0838799998164177, -0.03550700098276138, 0.9958429932594299, 0.0934389978647232, -0.0012799999676644802, 0.9956240057945251, -0.629721999168396, 0.2604379951953888, 0.7318620085716248, -0.6821659803390503, 0.0003429999924264848, 0.7311969995498657, -0.803725004196167, 0.3325839936733246, 0.4933690130710602, -0.8703219890594482, 0.00005400000009103678, 0.492482990026474, 0.6834070086479187, -0.6834070086479187, -0.2567310035228729, 0.6835309863090515, -0.6835309863090515, 0.25606799125671387, 0.06492599844932556, -0.06492500007152557, 0.9957759976387024, -0.48139700293540955, 0.48139700293540955, 0.7324709892272949, -0.6148040294647217, 0.6148040294647217, 0.4939970076084137, 0.3698819875717163, -0.8930140137672424, -0.2563450038433075, 0.36910200119018555, -0.8934370279312134, 0.2559959888458252, 0.03550700098276138, -0.0838790014386177, 0.9958429932594299, -0.26043900847435, 0.6297230124473572, 0.7318609952926636, -0.3325839936733246, 0.803725004196167, 0.4933690130710602, 0, -0.9667419791221619, -0.25575199723243713, 0, -0.9668239951133728, 0.2554430067539215, 0, -0.09205400198698044, 0.9957540035247803, 0, 0.6820489764213562, 0.7313070297241211, 0, 0.8703010082244873, 0.4925200045108795, 0, -0.9667419791221619, -0.25575199723243713, -0.3698819875717163, -0.8930140137672424, -0.2563450038433075, -0.36910200119018555, -0.8934370279312134, 0.2559970021247864, 0, -0.9668239951133728, 0.2554430067539215, -0.03550700098276138, -0.0838799998164177, 0.9958429932594299, 0, -0.09205400198698044, 0.9957540035247803, 0.2604379951953888, 0.629721999168396, 0.7318620085716248, 0, 0.6820489764213562, 0.7313070297241211, 0.3325839936733246, 0.803725004196167, 0.4933690130710602, 0, 0.8703010082244873, 0.4925200045108795, -0.6834070086479187, -0.6834070086479187, -0.2567310035228729, -0.6835309863090515, -0.6835309863090515, 0.25606799125671387, -0.06492500007152557, -0.06492599844932556, 0.9957759976387024, 0.48139700293540955, 0.48139700293540955, 0.7324709892272949, 0.6148040294647217, 0.6148040294647217, 0.4939970076084137, -0.8930140137672424, -0.3698819875717163, -0.2563450038433075, -0.8934370279312134, -0.36910200119018555, 0.2559959888458252, -0.0838790014386177, -0.03550700098276138, 0.9958429932594299, 0.6297230124473572, 0.26043900847435, 0.7318609952926636, 0.803725004196167, 0.3325839936733246, 0.4933690130710602, -0.9667419791221619, 0, -0.25575199723243713, -0.9668239951133728, 0, 0.2554430067539215, -0.09205400198698044, 0, 0.9957540035247803, 0.6820489764213562, 0, 0.7313070297241211, 0.8703010082244873, 0, 0.4925200045108795, 0.8703010082244873, 0, 0.4925200045108795, 0.803725004196167, -0.3325839936733246, 0.4933690130710602, 0.8454390168190002, -0.34983500838279724, 0.40354499220848083, 0.9153209924697876, 0, 0.4027250111103058, 0.8699960112571716, -0.36004599928855896, 0.33685898780822754, 0.9418079853057861, 0, 0.33615100383758545, 0.9041929841041565, -0.37428000569343567, 0.20579099655151367, 0.9786900281906128, 0, 0.20534199476242065, 0.9218789935112, -0.38175201416015625, -0.06636899709701538, 0.9978039860725403, 0, -0.06623899936676025, 0.6148040294647217, -0.6148040294647217, 0.4939970076084137, 0.6468020081520081, -0.6468020081520081, 0.40409600734710693, 0.6656550168991089, -0.6656550168991089, 0.3373520076274872, 0.6919230222702026, -0.6919230222702026, 0.20611999928951263, 0.7055429816246033, -0.7055429816246033, -0.06647899746894836, 0.3325839936733246, -0.803725004196167, 0.4933690130710602, 0.34983500838279724, -0.8454390168190002, 0.40354499220848083, 0.36004701256752014, -0.8699960112571716, 0.33685800433158875, 0.37428000569343567, -0.9041929841041565, 0.20579099655151367, 0.38175201416015625, -0.9218789935112, -0.06636899709701538, 0, -0.8703010082244873, 0.4925200045108795, 0, -0.9153209924697876, 0.4027250111103058, 0, -0.9418079853057861, 0.33615100383758545, 0, -0.9786900281906128, 0.20534199476242065, 0, -0.9978039860725403, -0.06623899936676025, 0, -0.8703010082244873, 0.4925200045108795, -0.33248499035835266, -0.8042709827423096, 0.4925459921360016, -0.34983500838279724, -0.8454390168190002, 0.40354499220848083, 0, -0.9153209924697876, 0.4027250111103058, -0.36004599928855896, -0.8699960112571716, 0.33685898780822754, 0, -0.9418079853057861, 0.33615100383758545, -0.37428000569343567, -0.9041929841041565, 0.20579099655151367, 0, -0.9786900281906128, 0.20534199476242065, -0.38175201416015625, -0.9218789935112, -0.06636899709701538, 0, -0.9978039860725403, -0.06623899936676025, -0.6152420043945312, -0.6154839992523193, 0.4926010072231293, -0.6468020081520081, -0.6468020081520081, 0.40409600734710693, -0.6656550168991089, -0.6656550168991089, 0.3373520076274872, -0.6919230222702026, -0.6919230222702026, 0.20611999928951263, -0.7055429816246033, -0.7055429816246033, -0.06647899746894836, -0.8041260242462158, -0.33270499110221863, 0.49263399839401245, -0.8454390168190002, -0.34983500838279724, 0.40354499220848083, -0.8699960112571716, -0.36004701256752014, 0.33685800433158875, -0.9041929841041565, -0.37428000569343567, 0.20579099655151367, -0.9218789935112, -0.38175201416015625, -0.06636899709701538, -0.8703219890594482, 0.00005400000009103678, 0.492482990026474, -0.9153209924697876, 0, 0.4027250111103058, -0.9418079853057861, 0, 0.33615100383758545, -0.9786900281906128, 0, 0.20534199476242065, -0.9978039860725403, 0, -0.06623899936676025, -0.8703219890594482, 0.00005400000009103678, 0.492482990026474, -0.803725004196167, 0.3325839936733246, 0.4933690130710602, -0.8454390168190002, 0.34983500838279724, 0.40354499220848083, -0.9153209924697876, 0, 0.4027250111103058, -0.8699960112571716, 0.36004599928855896, 0.33685898780822754, -0.9418079853057861, 0, 0.33615100383758545, -0.9041929841041565, 0.37428000569343567, 0.20579099655151367, -0.9786900281906128, 0, 0.20534199476242065, -0.9218789935112, 0.38175201416015625, -0.06636899709701538, -0.9978039860725403, 0, -0.06623899936676025, -0.6148040294647217, 0.6148040294647217, 0.4939970076084137, -0.6468020081520081, 0.6468020081520081, 0.40409600734710693, -0.6656550168991089, 0.6656550168991089, 0.3373520076274872, -0.6919230222702026, 0.6919230222702026, 0.20611999928951263, -0.7055429816246033, 0.7055429816246033, -0.06647899746894836, -0.3325839936733246, 0.803725004196167, 0.4933690130710602, -0.34983500838279724, 0.8454390168190002, 0.40354499220848083, -0.36004701256752014, 0.8699960112571716, 0.33685800433158875, -0.37428000569343567, 0.9041929841041565, 0.20579099655151367, -0.38175201416015625, 0.9218789935112, -0.06636899709701538, 0, 0.8703010082244873, 0.4925200045108795, 0, 0.9153209924697876, 0.4027250111103058, 0, 0.9418079853057861, 0.33615100383758545, 0, 0.9786900281906128, 0.20534199476242065, 0, 0.9978039860725403, -0.06623899936676025, 0, 0.8703010082244873, 0.4925200045108795, 0.3325839936733246, 0.803725004196167, 0.4933690130710602, 0.34983500838279724, 0.8454390168190002, 0.40354499220848083, 0, 0.9153209924697876, 0.4027250111103058, 0.36004599928855896, 0.8699960112571716, 0.33685898780822754, 0, 0.9418079853057861, 0.33615100383758545, 0.37428000569343567, 0.9041929841041565, 0.20579099655151367, 0, 0.9786900281906128, 0.20534199476242065, 0.38175201416015625, 0.9218789935112, -0.06636899709701538, 0, 0.9978039860725403, -0.06623899936676025, 0.6148040294647217, 0.6148040294647217, 0.4939970076084137, 0.6468020081520081, 0.6468020081520081, 0.40409600734710693, 0.6656550168991089, 0.6656550168991089, 0.3373520076274872, 0.6919230222702026, 0.6919230222702026, 0.20611999928951263, 0.7055429816246033, 0.7055429816246033, -0.06647899746894836, 0.803725004196167, 0.3325839936733246, 0.4933690130710602, 0.8454390168190002, 0.34983500838279724, 0.40354499220848083, 0.8699960112571716, 0.36004701256752014, 0.33685800433158875, 0.9041929841041565, 0.37428000569343567, 0.20579099655151367, 0.9218789935112, 0.38175201416015625, -0.06636899709701538, 0.8703010082244873, 0, 0.4925200045108795, 0.9153209924697876, 0, 0.4027250111103058, 0.9418079853057861, 0, 0.33615100383758545, 0.9786900281906128, 0, 0.20534199476242065, 0.9978039860725403, 0, -0.06623899936676025, 0.9978039860725403, 0, -0.06623899936676025, 0.9218789935112, -0.38175201416015625, -0.06636899709701538, 0.8314369916915894, -0.3441790044307709, -0.4361799955368042, 0.9001820087432861, 0, -0.4355129897594452, 0.6735119819641113, -0.2785939872264862, -0.6846650242805481, 0.7296109795570374, 0, -0.6838629841804504, 0.6403989791870117, -0.26487401127815247, -0.7209240198135376, 0.6939510107040405, 0, -0.7200220227241516, 0.7329490184783936, -0.303166002035141, -0.6089959740638733, 0.7939500212669373, 0, -0.6079840064048767, 0.7055429816246033, -0.7055429816246033, -0.06647899746894836, 0.6360920071601868, -0.6360920071601868, -0.4367780089378357, 0.5149649977684021, -0.5149649977684021, -0.6852890253067017, 0.48965099453926086, -0.48965099453926086, -0.7214459776878357, 0.5605549812316895, -0.5605549812316895, -0.6095539927482605, 0.38175201416015625, -0.9218789935112, -0.06636899709701538, 0.3441790044307709, -0.8314369916915894, -0.4361799955368042, 0.2785939872264862, -0.6735119819641113, -0.6846650242805481, 0.26487401127815247, -0.6403989791870117, -0.7209240198135376, 0.303166002035141, -0.7329490184783936, -0.6089959740638733, 0, -0.9978039860725403, -0.06623899936676025, 0, -0.9001820087432861, -0.4355129897594452, 0, -0.7296109795570374, -0.6838629841804504, 0, -0.6939510107040405, -0.7200220227241516, 0, -0.7939500212669373, -0.6079840064048767, 0, -0.9978039860725403, -0.06623899936676025, -0.38175201416015625, -0.9218789935112, -0.06636899709701538, -0.3441790044307709, -0.8314369916915894, -0.4361799955368042, 0, -0.9001820087432861, -0.4355129897594452, -0.2785939872264862, -0.6735119819641113, -0.6846650242805481, 0, -0.7296109795570374, -0.6838629841804504, -0.26487401127815247, -0.6403989791870117, -0.7209240198135376, 0, -0.6939510107040405, -0.7200220227241516, -0.303166002035141, -0.7329490184783936, -0.6089959740638733, 0, -0.7939500212669373, -0.6079840064048767, -0.7055429816246033, -0.7055429816246033, -0.06647899746894836, -0.6360920071601868, -0.6360920071601868, -0.4367780089378357, -0.5149649977684021, -0.5149649977684021, -0.6852890253067017, -0.48965099453926086, -0.48965099453926086, -0.7214459776878357, -0.5605549812316895, -0.5605549812316895, -0.6095539927482605, -0.9218789935112, -0.38175201416015625, -0.06636899709701538, -0.8314369916915894, -0.3441790044307709, -0.4361799955368042, -0.6735119819641113, -0.2785939872264862, -0.6846650242805481, -0.6403989791870117, -0.26487401127815247, -0.7209240198135376, -0.7329490184783936, -0.303166002035141, -0.6089959740638733, -0.9978039860725403, 0, -0.06623899936676025, -0.9001820087432861, 0, -0.4355129897594452, -0.7296109795570374, 0, -0.6838629841804504, -0.6939510107040405, 0, -0.7200220227241516, -0.7939500212669373, 0, -0.6079840064048767, -0.9978039860725403, 0, -0.06623899936676025, -0.9218789935112, 0.38175201416015625, -0.06636899709701538, -0.8314369916915894, 0.3441790044307709, -0.4361799955368042, -0.9001820087432861, 0, -0.4355129897594452, -0.6735119819641113, 0.2785939872264862, -0.6846650242805481, -0.7296109795570374, 0, -0.6838629841804504, -0.6403989791870117, 0.26487401127815247, -0.7209240198135376, -0.6939510107040405, 0, -0.7200220227241516, -0.7329490184783936, 0.303166002035141, -0.6089959740638733, -0.7939500212669373, 0, -0.6079840064048767, -0.7055429816246033, 0.7055429816246033, -0.06647899746894836, -0.6360920071601868, 0.6360920071601868, -0.4367780089378357, -0.5149649977684021, 0.5149649977684021, -0.6852890253067017, -0.48965099453926086, 0.48965099453926086, -0.7214459776878357, -0.5605549812316895, 0.5605549812316895, -0.6095539927482605, -0.38175201416015625, 0.9218789935112, -0.06636899709701538, -0.3441790044307709, 0.8314369916915894, -0.4361799955368042, -0.2785939872264862, 0.6735119819641113, -0.6846650242805481, -0.26487401127815247, 0.6403989791870117, -0.7209240198135376, -0.303166002035141, 0.7329490184783936, -0.6089959740638733, 0, 0.9978039860725403, -0.06623899936676025, 0, 0.9001820087432861, -0.4355129897594452, 0, 0.7296109795570374, -0.6838629841804504, 0, 0.6939510107040405, -0.7200220227241516, 0, 0.7939500212669373, -0.6079840064048767, 0, 0.9978039860725403, -0.06623899936676025, 0.38175201416015625, 0.9218789935112, -0.06636899709701538, 0.3441790044307709, 0.8314369916915894, -0.4361799955368042, 0, 0.9001820087432861, -0.4355129897594452, 0.2785939872264862, 0.6735119819641113, -0.6846650242805481, 0, 0.7296109795570374, -0.6838629841804504, 0.26487401127815247, 0.6403989791870117, -0.7209240198135376, 0, 0.6939510107040405, -0.7200220227241516, 0.303166002035141, 0.7329490184783936, -0.6089959740638733, 0, 0.7939500212669373, -0.6079840064048767, 0.7055429816246033, 0.7055429816246033, -0.06647899746894836, 0.6360920071601868, 0.6360920071601868, -0.4367780089378357, 0.5149649977684021, 0.5149649977684021, -0.6852890253067017, 0.48965099453926086, 0.48965099453926086, -0.7214459776878357, 0.5605549812316895, 0.5605549812316895, -0.6095539927482605, 0.9218789935112, 0.38175201416015625, -0.06636899709701538, 0.8314369916915894, 0.3441790044307709, -0.4361799955368042, 0.6735119819641113, 0.2785939872264862, -0.6846650242805481, 0.6403989791870117, 0.26487401127815247, -0.7209240198135376, 0.7329490184783936, 0.303166002035141, -0.6089959740638733, 0.9978039860725403, 0, -0.06623899936676025, 0.9001820087432861, 0, -0.4355129897594452, 0.7296109795570374, 0, -0.6838629841804504, 0.6939510107040405, 0, -0.7200220227241516, 0.7939500212669373, 0, -0.6079840064048767, 0.7939500212669373, 0, -0.6079840064048767, 0.7329490184783936, -0.303166002035141, -0.6089959740638733, 0.576229989528656, -0.23821599781513214, -0.7818009853363037, 0.6238600015640259, 0, -0.7815359830856323, 0.16362899541854858, -0.06752700358629227, -0.9842079877853394, 0.17729100584983826, 0, -0.984158992767334, 0.04542100057005882, -0.018735000863671303, -0.9987919926643372, 0.04920699819922447, 0, -0.9987890124320984, 0, 0, -1, 0, 0, -1, 0.5605549812316895, -0.5605549812316895, -0.6095539927482605, 0.44041600823402405, -0.44041600823402405, -0.7823479771614075, 0.12490200251340866, -0.12490200251340866, -0.9842759966850281, 0.034662000834941864, -0.034662000834941864, -0.9987980127334595, 0, 0, -1, 0.303166002035141, -0.7329490184783936, -0.6089959740638733, 0.23821599781513214, -0.576229989528656, -0.7818009853363037, 0.06752700358629227, -0.16362899541854858, -0.9842079877853394, 0.018735000863671303, -0.04542100057005882, -0.9987919926643372, 0, 0, -1, 0, -0.7939500212669373, -0.6079840064048767, 0, -0.6238600015640259, -0.7815359830856323, 0, -0.17729100584983826, -0.984158992767334, 0, -0.04920699819922447, -0.9987890124320984, 0, 0, -1, 0, -0.7939500212669373, -0.6079840064048767, -0.303166002035141, -0.7329490184783936, -0.6089959740638733, -0.23821599781513214, -0.576229989528656, -0.7818009853363037, 0, -0.6238600015640259, -0.7815359830856323, -0.06752700358629227, -0.16362899541854858, -0.9842079877853394, 0, -0.17729100584983826, -0.984158992767334, -0.018735000863671303, -0.04542100057005882, -0.9987919926643372, 0, -0.04920699819922447, -0.9987890124320984, 0, 0, -1, 0, 0, -1, -0.5605549812316895, -0.5605549812316895, -0.6095539927482605, -0.44041600823402405, -0.44041600823402405, -0.7823479771614075, -0.12490200251340866, -0.12490200251340866, -0.9842759966850281, -0.034662000834941864, -0.034662000834941864, -0.9987980127334595, 0, 0, -1, -0.7329490184783936, -0.303166002035141, -0.6089959740638733, -0.576229989528656, -0.23821599781513214, -0.7818009853363037, -0.16362899541854858, -0.06752700358629227, -0.9842079877853394, -0.04542100057005882, -0.018735000863671303, -0.9987919926643372, 0, 0, -1, -0.7939500212669373, 0, -0.6079840064048767, -0.6238600015640259, 0, -0.7815359830856323, -0.17729100584983826, 0, -0.984158992767334, -0.04920699819922447, 0, -0.9987890124320984, 0, 0, -1, -0.7939500212669373, 0, -0.6079840064048767, -0.7329490184783936, 0.303166002035141, -0.6089959740638733, -0.576229989528656, 0.23821599781513214, -0.7818009853363037, -0.6238600015640259, 0, -0.7815359830856323, -0.16362899541854858, 0.06752700358629227, -0.9842079877853394, -0.17729100584983826, 0, -0.984158992767334, -0.04542100057005882, 0.018735000863671303, -0.9987919926643372, -0.04920699819922447, 0, -0.9987890124320984, 0, 0, -1, 0, 0, -1, -0.5605549812316895, 0.5605549812316895, -0.6095539927482605, -0.44041600823402405, 0.44041600823402405, -0.7823479771614075, -0.12490200251340866, 0.12490200251340866, -0.9842759966850281, -0.034662000834941864, 0.034662000834941864, -0.9987980127334595, 0, 0, -1, -0.303166002035141, 0.7329490184783936, -0.6089959740638733, -0.23821599781513214, 0.576229989528656, -0.7818009853363037, -0.06752700358629227, 0.16362899541854858, -0.9842079877853394, -0.018735000863671303, 0.04542100057005882, -0.9987919926643372, 0, 0, -1, 0, 0.7939500212669373, -0.6079840064048767, 0, 0.6238600015640259, -0.7815359830856323, 0, 0.17729100584983826, -0.984158992767334, 0, 0.04920699819922447, -0.9987890124320984, 0, 0, -1, 0, 0.7939500212669373, -0.6079840064048767, 0.303166002035141, 0.7329490184783936, -0.6089959740638733, 0.23821599781513214, 0.576229989528656, -0.7818009853363037, 0, 0.6238600015640259, -0.7815359830856323, 0.06752700358629227, 0.16362899541854858, -0.9842079877853394, 0, 0.17729100584983826, -0.984158992767334, 0.018735000863671303, 0.04542100057005882, -0.9987919926643372, 0, 0.04920699819922447, -0.9987890124320984, 0, 0, -1, 0, 0, -1, 0.5605549812316895, 0.5605549812316895, -0.6095539927482605, 0.44041600823402405, 0.44041600823402405, -0.7823479771614075, 0.12490200251340866, 0.12490200251340866, -0.9842759966850281, 0.034662000834941864, 0.034662000834941864, -0.9987980127334595, 0, 0, -1, 0.7329490184783936, 0.303166002035141, -0.6089959740638733, 0.576229989528656, 0.23821599781513214, -0.7818009853363037, 0.16362899541854858, 0.06752700358629227, -0.9842079877853394, 0.04542100057005882, 0.018735000863671303, -0.9987919926643372, 0, 0, -1, 0.7939500212669373, 0, -0.6079840064048767, 0.6238600015640259, 0, -0.7815359830856323, 0.17729100584983826, 0, -0.984158992767334, 0.04920699819922447, 0, -0.9987890124320984, 0, 0, -1, 0.007784999907016754, 0.00021499999274965376, -0.999970018863678, 0.007038000039756298, -0.5829259753227234, -0.8124949932098389, 0.0361270010471344, -0.5456140041351318, -0.837257981300354, 0.03913800045847893, 0.0009879999561235309, -0.9992330074310303, 0.16184599697589874, -0.5630490183830261, -0.8104209899902344, 0.17951199412345886, 0.0043680001981556416, -0.9837459921836853, 0.4823650121688843, -0.6427459716796875, -0.5951480269432068, 0.6122999787330627, 0.010459000244736671, -0.790556013584137, 0.7387199997901917, -0.6641989946365356, -0.11459299921989441, 0.9861519932746887, 0.006668999791145325, -0.16570700705051422, -0.0019079999765381217, -0.9867690205574036, 0.1621209979057312, 0.002761000068858266, -0.9998499751091003, 0.017105000093579292, 0.010532000102102757, -0.9972469806671143, 0.07339800149202347, -0.06604000180959702, -0.9893029928207397, 0.13006900250911713, -0.09442699700593948, -0.9953929781913757, 0.016594000160694122, -0.009201999753713608, -0.4902929961681366, 0.8715090155601501, -0.04860600084066391, -0.5394579768180847, 0.8406090140342712, -0.22329799830913544, -0.5527390241622925, 0.8028810024261475, -0.5963649749755859, -0.5751349925994873, 0.5599709749221802, -0.8033369779586792, -0.5916029810905457, 0.06823500245809555, -0.01056000031530857, -0.00010299999848939478, 0.9999439716339111, -0.05879800021648407, -0.0007089999853633344, 0.9982699751853943, -0.28071001172065735, -0.0032679999712854624, 0.9597870111465454, -0.7497230172157288, -0.004267000127583742, 0.6617379784584045, -0.9973509907722473, -0.0020580000709742308, 0.07271400094032288, -0.01056000031530857, -0.00010299999848939478, 0.9999439716339111, -0.008791999891400337, 0.49032899737358093, 0.8714929819107056, -0.04649300128221512, 0.5387560129165649, 0.8411779999732971, -0.05879800021648407, -0.0007089999853633344, 0.9982699751853943, -0.21790899336338043, 0.5491610169410706, 0.8068069815635681, -0.28071001172065735, -0.0032679999712854624, 0.9597870111465454, -0.5972909927368164, 0.5741199851036072, 0.560027003288269, -0.7497230172157288, -0.004267000127583742, 0.6617379784584045, -0.8040000200271606, 0.5912910103797913, 0.0629120022058487, -0.9973509907722473, -0.0020580000709742308, 0.07271400094032288, -0.0018050000071525574, 0.986840009689331, 0.16169099509716034, 0.0020310000982135534, 0.999891996383667, 0.014553000219166279, 0.009215000085532665, 0.9981520175933838, 0.060068998485803604, -0.059335000813007355, 0.9917230010032654, 0.11386600136756897, -0.08690100163221359, 0.9961410164833069, 0.01228999998420477, 0.006417000200599432, 0.5830950140953064, -0.812379002571106, 0.03378299996256828, 0.5453730225563049, -0.8375130295753479, 0.1571130007505417, 0.562188982963562, -0.8119469881057739, 0.4844059944152832, 0.6465290188789368, -0.5893650054931641, 0.7388700246810913, 0.6661880016326904, -0.10131999850273132, 0.007784999907016754, 0.00021499999274965376, -0.999970018863678, 0.03913800045847893, 0.0009879999561235309, -0.9992330074310303, 0.17951199412345886, 0.0043680001981556416, -0.9837459921836853, 0.6122999787330627, 0.010459000244736671, -0.790556013584137, 0.9861519932746887, 0.006668999791145325, -0.16570700705051422, 0.9861519932746887, 0.006668999791145325, -0.16570700705051422, 0.7387199997901917, -0.6641989946365356, -0.11459299921989441, 0.7256090044975281, -0.6373609900474548, 0.25935098528862, 0.94651198387146, 0.0033569999504834414, 0.3226499855518341, 0.6459450125694275, -0.6077200174331665, 0.46198800206184387, 0.8258299827575684, 0.007451999932527542, 0.5638700127601624, 0.5316150188446045, -0.5586140155792236, 0.6366599798202515, 0.6500110030174255, 0.006936000194400549, 0.759893000125885, 0.4249640107154846, -0.5955389738082886, 0.6817179918289185, 0.5324289798736572, 0.005243999883532524, 0.8464580178260803, -0.09442699700593948, -0.9953929781913757, 0.016594000160694122, -0.04956100136041641, -0.9985759854316711, -0.01975500024855137, -0.03781700134277344, -0.998649001121521, -0.035624999552965164, -0.0379129983484745, -0.9986140131950378, -0.03651199862360954, -0.1688539981842041, -0.9395300149917603, -0.2979460060596466, -0.8033369779586792, -0.5916029810905457, 0.06823500245809555, -0.7423409819602966, -0.5995240211486816, -0.2991659939289093, -0.6196020245552063, -0.5795029997825623, -0.5294060111045837, -0.483707994222641, -0.5438370108604431, -0.6857600212097168, -0.44529199600219727, -0.4131770133972168, -0.7943549752235413, -0.9973509907722473, -0.0020580000709742308, 0.07271400094032288, -0.9265130162239075, -0.0019950000569224358, -0.3762570023536682, -0.7539200186729431, -0.004317000042647123, -0.6569520235061646, -0.5662239789962769, -0.003461000043898821, -0.8242440223693848, -0.4818040132522583, -0.0018500000005587935, -0.8762770295143127, -0.9973509907722473, -0.0020580000709742308, 0.07271400094032288, -0.8040000200271606, 0.5912910103797913, 0.0629120022058487, -0.7446749806404114, 0.5989770293235779, -0.29442399740219116, -0.9265130162239075, -0.0019950000569224358, -0.3762570023536682, -0.6219490170478821, 0.5781649947166443, -0.5281140208244324, -0.7539200186729431, -0.004317000042647123, -0.6569520235061646, -0.48117101192474365, 0.5428280234336853, -0.6883400082588196, -0.5662239789962769, -0.003461000043898821, -0.8242440223693848, -0.43805500864982605, 0.41574400663375854, -0.7970349788665771, -0.4818040132522583, -0.0018500000005587935, -0.8762770295143127, -0.08690100163221359, 0.9961410164833069, 0.01228999998420477, -0.04433799907565117, 0.9988710284233093, -0.017055999487638474, -0.026177000254392624, 0.9992600083351135, -0.02816700004041195, -0.025293000042438507, 0.9992780089378357, -0.028332000598311424, -0.15748199820518494, 0.9441670179367065, -0.28939300775527954, 0.7388700246810913, 0.6661880016326904, -0.10131999850273132, 0.7282440066337585, 0.63714200258255, 0.25240999460220337, 0.6470540165901184, 0.6082550287246704, 0.4597249925136566, 0.5229939818382263, 0.5621700286865234, 0.6406570076942444, 0.4099780023097992, 0.6046689748764038, 0.6828569769859314, 0.9861519932746887, 0.006668999791145325, -0.16570700705051422, 0.94651198387146, 0.0033569999504834414, 0.3226499855518341, 0.8258299827575684, 0.007451999932527542, 0.5638700127601624, 0.6500110030174255, 0.006936000194400549, 0.759893000125885, 0.5324289798736572, 0.005243999883532524, 0.8464580178260803, -0.230786994099617, 0.006523000076413155, 0.9729819893836975, -0.15287800133228302, -0.7101899981498718, 0.6872109770774841, -0.31672099232673645, -0.7021129727363586, 0.6377500295639038, -0.5489360094070435, 0.0015109999803826213, 0.8358629941940308, -0.6010670065879822, -0.645330011844635, 0.471451997756958, -0.8756710290908813, -0.009891999885439873, 0.4828070104122162, -0.635890007019043, -0.629800021648407, 0.4460900127887726, -0.8775539994239807, -0.01909100078046322, 0.47909700870513916, -0.4357450008392334, -0.670009970664978, 0.6010090112686157, -0.6961889863014221, -0.02449600026011467, 0.7174400091171265, 0.11111299693584442, -0.9901599884033203, -0.08506900072097778, 0.22330999374389648, -0.9747260212898254, 0.006539999973028898, 0.19009700417518616, -0.9694579839706421, 0.15496399998664856, 0.005270000081509352, -0.9818699955940247, 0.18948200345039368, -0.011750999838113785, -0.9690240025520325, 0.24668699502944946, 0.3439059853553772, -0.5994120240211487, -0.7227950096130371, 0.5724899768829346, -0.5916270017623901, -0.5676559805870056, 0.7874360084533691, -0.5605109930038452, -0.2564600110054016, 0.6470969915390015, -0.6981409788131714, -0.3063740134239197, 0.4275279939174652, -0.7535750269889832, -0.49934399127960205, 0.4109260141849518, -0.0012839999981224537, -0.9116680026054382, 0.6715199947357178, 0.0008989999769255519, -0.7409859895706177, 0.9220259785652161, 0.00725199980661273, -0.3870599865913391, 0.8469099998474121, 0.01385399978607893, -0.5315560102462769, 0.5359240174293518, 0.010503999888896942, -0.8442010283470154, 0.4109260141849518, -0.0012839999981224537, -0.9116680026054382, 0.3411880135536194, 0.6009309887886047, -0.7228230237960815, 0.5786640048027039, 0.591838002204895, -0.5611389875411987, 0.6715199947357178, 0.0008989999769255519, -0.7409859895706177, 0.7848690152168274, 0.5665420293807983, -0.25102001428604126, 0.9220259785652161, 0.00725199980661273, -0.3870599865913391, 0.6426810026168823, 0.7039899826049805, -0.3022570013999939, 0.8469099998474121, 0.01385399978607893, -0.5315560102462769, 0.4185889959335327, 0.7581170201301575, -0.5000420212745667, 0.5359240174293518, 0.010503999888896942, -0.8442010283470154, 0.11580599844455719, 0.9901139736175537, -0.07913900166749954, 0.23281100392341614, 0.9724410176277161, 0.012564999982714653, 0.20666299760341644, 0.9662799835205078, 0.15360000729560852, 0.02449899911880493, 0.9865779876708984, 0.16144299507141113, 0.0033809999004006386, 0.9774550199508667, 0.2111150026321411, -0.13491199910640717, 0.7135509848594666, 0.6874909996986389, -0.31953999400138855, 0.7050619721412659, 0.6330729722976685, -0.6039019823074341, 0.6499029994010925, 0.4614419937133789, -0.6318150162696838, 0.6400719881057739, 0.43716898560523987, -0.4243049919605255, 0.6667500138282776, 0.6127070188522339, -0.230786994099617, 0.006523000076413155, 0.9729819893836975, -0.5489360094070435, 0.0015109999803826213, 0.8358629941940308, -0.8756710290908813, -0.009891999885439873, 0.4828070104122162, -0.8775539994239807, -0.01909100078046322, 0.47909700870513916, -0.6961889863014221, -0.02449600026011467, 0.7174400091171265, -0.6961889863014221, -0.02449600026011467, 0.7174400091171265, -0.4357450008392334, -0.670009970664978, 0.6010090112686157, -0.25985801219940186, -0.5525479912757874, 0.7919380068778992, -0.42579901218414307, -0.010804999619722366, 0.9047530293464661, 0.009537000209093094, 0.021669000387191772, 0.9997199773788452, 0.022041000425815582, -0.001623000018298626, 0.9997559785842896, 0.4101540148258209, 0.8490809798240662, 0.3329179883003235, 0.9995980262756348, -0.01155600044876337, 0.02587899938225746, 0.5415220260620117, 0.6370009779930115, -0.5486199855804443, 0.7095860242843628, -0.009670999832451344, -0.7045519948005676, -0.011750999838113785, -0.9690240025520325, 0.24668699502944946, 0.046310000121593475, -0.8891720175743103, 0.45522499084472656, -0.010688000358641148, -0.14889900386333466, 0.9887949824333191, -0.04437499865889549, 0.7291200160980225, 0.6829460263252258, 0.12282499670982361, 0.9923850297927856, 0.009232000447809696, 0.4275279939174652, -0.7535750269889832, -0.49934399127960205, 0.48183900117874146, -0.857479989528656, -0.18044300377368927, 0.45527198910713196, -0.49992498755455017, 0.7367510199546814, -0.22054199874401093, 0.3582780063152313, 0.9071930050849915, -0.23591899871826172, 0.7157959938049316, 0.6572499871253967, 0.5359240174293518, 0.010503999888896942, -0.8442010283470154, 0.7280910015106201, 0.015584999695420265, -0.6853029727935791, 0.8887389898300171, 0.016679000109434128, 0.4581089913845062, -0.26009801030158997, -0.0007999999797903001, 0.965582013130188, -0.37161099910736084, 0.004416999872773886, 0.9283779859542847, 0.5359240174293518, 0.010503999888896942, -0.8442010283470154, 0.4185889959335327, 0.7581170201301575, -0.5000420212745667, 0.4801650047302246, 0.8588529825210571, -0.17836299538612366, 0.7280910015106201, 0.015584999695420265, -0.6853029727935791, 0.4881030023097992, 0.49794700741767883, 0.7168020009994507, 0.8887389898300171, 0.016679000109434128, 0.4581089913845062, -0.2220049947500229, -0.36189401149749756, 0.9053990244865417, -0.26009801030158997, -0.0007999999797903001, 0.965582013130188, -0.23540399968624115, -0.7104769945144653, 0.6631799936294556, -0.37161099910736084, 0.004416999872773886, 0.9283779859542847, 0.0033809999004006386, 0.9774550199508667, 0.2111150026321411, 0.058719001710414886, 0.8971999883651733, 0.437703013420105, 0.0013249999610707164, 0.164000004529953, 0.9864590167999268, -0.04418899863958359, -0.7303190231323242, 0.6816750168800354, 0.13880200684070587, -0.9897300004959106, -0.034189000725746155, -0.4243049919605255, 0.6667500138282776, 0.6127070188522339, -0.25888898968696594, 0.5453789830207825, 0.7972059845924377, 0.012268000282347202, -0.01928500086069107, 0.9997389912605286, 0.3986299932003021, -0.8456630110740662, 0.3548929989337921, 0.5375639796257019, -0.6107370257377625, -0.5813990235328674, -0.6961889863014221, -0.02449600026011467, 0.7174400091171265, -0.42579901218414307, -0.010804999619722366, 0.9047530293464661, 0.022041000425815582, -0.001623000018298626, 0.9997559785842896, 0.9995980262756348, -0.01155600044876337, 0.02587899938225746, 0.7095860242843628, -0.009670999832451344, -0.7045519948005676, 0, 0, 1, 0, 0, 1, 0.7626410126686096, -0.31482499837875366, 0.5650339722633362, 0.8245400190353394, -0.00001700000029813964, 0.5658029913902283, 0.8479819893836975, -0.3500339984893799, -0.39799800515174866, 0.917701005935669, -0.00003300000025774352, -0.397271990776062, 0.8641409873962402, -0.35644200444221497, -0.3552600145339966, 0.9352689981460571, -0.00011200000153621659, -0.3539389967918396, 0.7209920287132263, -0.29793301224708557, 0.6256250143051147, 0.7807120084762573, -0.00007500000356230885, 0.6248909831047058, 0, 0, 1, 0.5833569765090942, -0.5833380222320557, 0.5651649832725525, 0.648485004901886, -0.6484479904174805, -0.3987259864807129, 0.6608719825744629, -0.6607480049133301, -0.35589399933815, 0.5518630146980286, -0.5517799854278564, 0.6252880096435547, 0, 0, 1, 0.31482499837875366, -0.762628972530365, 0.5650510191917419, 0.35004499554634094, -0.8479880094528198, -0.39797601103782654, 0.35647401213645935, -0.8641520142555237, -0.35519900918006897, 0.29798200726509094, -0.7210670113563538, 0.6255149841308594, 0, 0, 1, -0.00001700000029813964, -0.8245400190353394, 0.5658029913902283, -0.00003300000025774352, -0.917701005935669, -0.397271990776062, -0.00011200000153621659, -0.9352689981460571, -0.3539389967918396, -0.00007500000356230885, -0.7807120084762573, 0.6248900294303894, 0, 0, 1, 0, 0, 1, -0.31482499837875366, -0.7626410126686096, 0.5650339722633362, -0.00001700000029813964, -0.8245400190353394, 0.5658029913902283, -0.3500339984893799, -0.8479819893836975, -0.39799800515174866, -0.00003300000025774352, -0.917701005935669, -0.397271990776062, -0.35644200444221497, -0.8641409873962402, -0.3552600145339966, -0.00011200000153621659, -0.9352689981460571, -0.3539389967918396, -0.29793301224708557, -0.7209920287132263, 0.6256250143051147, -0.00007500000356230885, -0.7807120084762573, 0.6248900294303894, 0, 0, 1, -0.5833380222320557, -0.5833569765090942, 0.5651649832725525, -0.6484479904174805, -0.648485004901886, -0.3987259864807129, -0.6607480049133301, -0.6608719825744629, -0.35589399933815, -0.5517799854278564, -0.5518630146980286, 0.6252880096435547, 0, 0, 1, -0.762628972530365, -0.31482499837875366, 0.5650510191917419, -0.8479880094528198, -0.35004499554634094, -0.39797601103782654, -0.8641520142555237, -0.35647401213645935, -0.35519900918006897, -0.7210670113563538, -0.29798200726509094, 0.6255149841308594, 0, 0, 1, -0.8245400190353394, 0.00001700000029813964, 0.5658029913902283, -0.917701005935669, 0.00003300000025774352, -0.397271990776062, -0.9352689981460571, 0.00011200000153621659, -0.3539389967918396, -0.7807120084762573, 0.00007500000356230885, 0.6248900294303894, 0, 0, 1, 0, 0, 1, -0.7626410126686096, 0.31482499837875366, 0.5650339722633362, -0.8245400190353394, 0.00001700000029813964, 0.5658029913902283, -0.8479819893836975, 0.3500339984893799, -0.39799800515174866, -0.917701005935669, 0.00003300000025774352, -0.397271990776062, -0.8641409873962402, 0.35644200444221497, -0.3552600145339966, -0.9352689981460571, 0.00011200000153621659, -0.3539389967918396, -0.7209920287132263, 0.29793301224708557, 0.6256250143051147, -0.7807120084762573, 0.00007500000356230885, 0.6248900294303894, 0, 0, 1, -0.5833569765090942, 0.5833380222320557, 0.5651649832725525, -0.648485004901886, 0.6484479904174805, -0.3987259864807129, -0.6608719825744629, 0.6607480049133301, -0.35589399933815, -0.5518630146980286, 0.5517799854278564, 0.6252880096435547, 0, 0, 1, -0.31482499837875366, 0.762628972530365, 0.5650510191917419, -0.35004499554634094, 0.8479880094528198, -0.39797601103782654, -0.35647401213645935, 0.8641520142555237, -0.35519900918006897, -0.29798200726509094, 0.7210670113563538, 0.6255149841308594, 0, 0, 1, 0.00001700000029813964, 0.8245400190353394, 0.5658029913902283, 0.00003300000025774352, 0.917701005935669, -0.397271990776062, 0.00011200000153621659, 0.9352689981460571, -0.3539389967918396, 0.00007500000356230885, 0.7807120084762573, 0.6248900294303894, 0, 0, 1, 0, 0, 1, 0.31482499837875366, 0.7626410126686096, 0.5650339722633362, 0.00001700000029813964, 0.8245400190353394, 0.5658029913902283, 0.3500339984893799, 0.8479819893836975, -0.39799800515174866, 0.00003300000025774352, 0.917701005935669, -0.397271990776062, 0.35644200444221497, 0.8641409873962402, -0.3552600145339966, 0.00011200000153621659, 0.9352689981460571, -0.3539389967918396, 0.29793301224708557, 0.7209920287132263, 0.6256250143051147, 0.00007500000356230885, 0.7807120084762573, 0.6248900294303894, 0, 0, 1, 0.5833380222320557, 0.5833569765090942, 0.5651649832725525, 0.6484479904174805, 0.648485004901886, -0.3987259864807129, 0.6607480049133301, 0.6608719825744629, -0.35589399933815, 0.5517799854278564, 0.5518630146980286, 0.6252880096435547, 0, 0, 1, 0.762628972530365, 0.31482499837875366, 0.5650510191917419, 0.8479880094528198, 0.35004499554634094, -0.39797601103782654, 0.8641520142555237, 0.35647401213645935, -0.35519900918006897, 0.7210670113563538, 0.29798200726509094, 0.6255149841308594, 0, 0, 1, 0.8245400190353394, -0.00001700000029813964, 0.5658029913902283, 0.917701005935669, -0.00003300000025774352, -0.397271990776062, 0.9352689981460571, -0.00011200000153621659, -0.3539389967918396, 0.7807120084762573, -0.00007500000356230885, 0.6248909831047058, 0.7807120084762573, -0.00007500000356230885, 0.6248909831047058, 0.7209920287132263, -0.29793301224708557, 0.6256250143051147, 0.21797800064086914, -0.0902160033583641, 0.9717749953269958, 0.23658299446105957, 0, 0.9716110229492188, 0.1595889925956726, -0.06596100330352783, 0.9849770069122314, 0.17308400571346283, 0, 0.9849069714546204, 0.3504979908466339, -0.1447400003671646, 0.9253119826316833, 0.37970298528671265, 0, 0.925108015537262, 0.48558899760246277, -0.20147399604320526, 0.8506529927253723, 0.5266720056533813, 0, 0.8500679731369019, 0.5518630146980286, -0.5517799854278564, 0.6252880096435547, 0.16663099825382233, -0.16663099825382233, 0.9718379974365234, 0.12190800160169601, -0.12190800160169601, 0.9850260019302368, 0.2676680088043213, -0.2676680088043213, 0.9255849719047546, 0.37131500244140625, -0.37131500244140625, 0.8510289788246155, 0.29798200726509094, -0.7210670113563538, 0.6255149841308594, 0.0902160033583641, -0.21797800064086914, 0.9717749953269958, 0.06596100330352783, -0.1595889925956726, 0.9849770069122314, 0.1447400003671646, -0.3504979908466339, 0.9253119826316833, 0.20147399604320526, -0.48558899760246277, 0.8506529927253723, -0.00007500000356230885, -0.7807120084762573, 0.6248900294303894, 0, -0.23658299446105957, 0.9716110229492188, 0, -0.17308400571346283, 0.9849069714546204, 0, -0.37970298528671265, 0.925108015537262, 0, -0.5266720056533813, 0.8500679731369019, -0.00007500000356230885, -0.7807120084762573, 0.6248900294303894, -0.29793301224708557, -0.7209920287132263, 0.6256250143051147, -0.0902160033583641, -0.21797800064086914, 0.9717749953269958, 0, -0.23658299446105957, 0.9716110229492188, -0.06596100330352783, -0.1595889925956726, 0.9849770069122314, 0, -0.17308400571346283, 0.9849069714546204, -0.1447400003671646, -0.3504979908466339, 0.9253119826316833, 0, -0.37970298528671265, 0.925108015537262, -0.20147399604320526, -0.48558899760246277, 0.8506529927253723, 0, -0.5266720056533813, 0.8500679731369019, -0.5517799854278564, -0.5518630146980286, 0.6252880096435547, -0.16663099825382233, -0.16663099825382233, 0.9718379974365234, -0.12190800160169601, -0.12190800160169601, 0.9850260019302368, -0.2676680088043213, -0.2676680088043213, 0.9255849719047546, -0.37131500244140625, -0.37131500244140625, 0.8510289788246155, -0.7210670113563538, -0.29798200726509094, 0.6255149841308594, -0.21797800064086914, -0.0902160033583641, 0.9717749953269958, -0.1595889925956726, -0.06596100330352783, 0.9849770069122314, -0.3504979908466339, -0.1447400003671646, 0.9253119826316833, -0.48558899760246277, -0.20147399604320526, 0.8506529927253723, -0.7807120084762573, 0.00007500000356230885, 0.6248900294303894, -0.23658299446105957, 0, 0.9716110229492188, -0.17308400571346283, 0, 0.9849069714546204, -0.37970298528671265, 0, 0.925108015537262, -0.5266720056533813, 0, 0.8500679731369019, -0.7807120084762573, 0.00007500000356230885, 0.6248900294303894, -0.7209920287132263, 0.29793301224708557, 0.6256250143051147, -0.21797800064086914, 0.0902160033583641, 0.9717749953269958, -0.23658299446105957, 0, 0.9716110229492188, -0.1595889925956726, 0.06596100330352783, 0.9849770069122314, -0.17308400571346283, 0, 0.9849069714546204, -0.3504979908466339, 0.1447400003671646, 0.9253119826316833, -0.37970298528671265, 0, 0.925108015537262, -0.48558899760246277, 0.20147399604320526, 0.8506529927253723, -0.5266720056533813, 0, 0.8500679731369019, -0.5518630146980286, 0.5517799854278564, 0.6252880096435547, -0.16663099825382233, 0.16663099825382233, 0.9718379974365234, -0.12190800160169601, 0.12190800160169601, 0.9850260019302368, -0.2676680088043213, 0.2676680088043213, 0.9255849719047546, -0.37131500244140625, 0.37131500244140625, 0.8510289788246155, -0.29798200726509094, 0.7210670113563538, 0.6255149841308594, -0.0902160033583641, 0.21797800064086914, 0.9717749953269958, -0.06596100330352783, 0.1595889925956726, 0.9849770069122314, -0.1447400003671646, 0.3504979908466339, 0.9253119826316833, -0.20147399604320526, 0.48558899760246277, 0.8506529927253723, 0.00007500000356230885, 0.7807120084762573, 0.6248900294303894, 0, 0.23658299446105957, 0.9716110229492188, 0, 0.17308400571346283, 0.9849069714546204, 0, 0.37970298528671265, 0.925108015537262, 0, 0.5266720056533813, 0.8500679731369019, 0.00007500000356230885, 0.7807120084762573, 0.6248900294303894, 0.29793301224708557, 0.7209920287132263, 0.6256250143051147, 0.0902160033583641, 0.21797800064086914, 0.9717749953269958, 0, 0.23658299446105957, 0.9716110229492188, 0.06596100330352783, 0.1595889925956726, 0.9849770069122314, 0, 0.17308400571346283, 0.9849069714546204, 0.1447400003671646, 0.3504979908466339, 0.9253119826316833, 0, 0.37970298528671265, 0.925108015537262, 0.20147399604320526, 0.48558899760246277, 0.8506529927253723, 0, 0.5266720056533813, 0.8500679731369019, 0.5517799854278564, 0.5518630146980286, 0.6252880096435547, 0.16663099825382233, 0.16663099825382233, 0.9718379974365234, 0.12190800160169601, 0.12190800160169601, 0.9850260019302368, 0.2676680088043213, 0.2676680088043213, 0.9255849719047546, 0.37131500244140625, 0.37131500244140625, 0.8510289788246155, 0.7210670113563538, 0.29798200726509094, 0.6255149841308594, 0.21797800064086914, 0.0902160033583641, 0.9717749953269958, 0.1595889925956726, 0.06596100330352783, 0.9849770069122314, 0.3504979908466339, 0.1447400003671646, 0.9253119826316833, 0.48558899760246277, 0.20147399604320526, 0.8506529927253723, 0.7807120084762573, -0.00007500000356230885, 0.6248909831047058, 0.23658299446105957, 0, 0.9716110229492188, 0.17308400571346283, 0, 0.9849069714546204, 0.37970298528671265, 0, 0.925108015537262, 0.5266720056533813, 0, 0.8500679731369019
  };
  core->addAttributeData(OGLConstants::NORMAL.location, normals,
      sizeof(normals), 3, GL_STATIC_DRAW);

  // define tangents
  const GLfloat tangents[] = {
      0.012897999957203865, 0.998727023601532, -0.048757001757621765, 0.3861910104751587, 0.9210079908370972, -0.016421999782323837, 0.38136398792266846, 0.9230089783668518, 0.000155999994603917, 0.012866999953985214, 0.9987300038337708, 0.04870200157165527, 0.3750790059566498, 0.9061710238456726, -0.0007169999880716205, 0.19210100173950195, 0.9812139868736267, 0.01775900088250637, 0.3782620131969452, 0.9142940044403076, -0.00011300000187475234, 0.10451500117778778, 0.9897350072860718, -0.09747499972581863, 0.3655939996242523, 0.9257190227508545, 0.028463000431656837, 0.04767199978232384, 0.9953050017356873, -0.08423800021409988, 0.7092679738998413, 0.7031199932098389, -0.016364000737667084, 0.7061989903450012, 0.7061989903450012, 0, 0.6937360167503357, 0.6937360167503357, 0, 0.6997770071029663, 0.6997770071029663, 0, 0.6924030184745789, 0.7150859832763672, 0.02822900004684925, 0.9243540167808533, 0.37810400128364563, -0.01657800003886223, 0.9230089783668518, 0.38136398792266846, -0.000155999994603917, 0.9061710238456726, 0.3750790059566498, 0.0007169999880716205, 0.9142940044403076, 0.3782620131969452, 0.00011300000187475234, 0.9133660197257996, 0.39544400572776794, 0.028490999713540077, 0.9987040162086487, 0.015853000804781914, 0.04836999997496605, 0.9987369775772095, 0.014649000018835068, -0.04806999862194061, 0.9812150001525879, 0.19211700558662415, -0.01754000037908554, 0.9897350072860718, 0.10452800244092941, 0.09745799750089645, 0.9953050017356873, 0.04767199978232384, 0.08423800021409988, 0.9988179802894592, -0.009758999571204185, -0.047600001096725464, 0.9094679951667786, -0.4095839858055115, -0.012636999599635601, 0.9240090250968933, -0.3811509907245636, -0.0003150000120513141, 0.9987890124320984, -0.01066299993544817, 0.04801800101995468, 0.9072269797325134, -0.37142300605773926, 0.0207310002297163, 0.9814350008964539, -0.19095200300216675, 0.01795700006186962, 0.914870023727417, -0.3771440088748932, -0.0011480000102892518, 0.989749014377594, -0.10442499816417694, -0.09742700308561325, 0.925815999507904, -0.3653950095176697, 0.028308000415563583, 0.9953050017356873, -0.04767199978232384, -0.08423800021409988, 0.6768929958343506, -0.7314029932022095, -0.01988700032234192, 0.6994619965553284, -0.7145140171051025, -0.00029799999902024865, 0.6940590143203735, -0.6933979988098145, 0.015560000203549862, 0.7002580165863037, -0.6996300220489502, -0.000783999974373728, 0.715142011642456, -0.6923869848251343, 0.028078999370336533, 0.351936012506485, -0.933899998664856, -0.019843999296426773, 0.36654001474380493, -0.9298419952392578, -0.0005210000090301037, 0.37116900086402893, -0.9084830284118652, 0.00152299995534122, 0.3776479959487915, -0.9147650003433228, -0.00011000000085914508, 0.39533698558807373, -0.9134349822998047, 0.028410999104380608, 0.0013210000470280647, -0.9989479780197144, 0.045830998569726944, 0.003897000104188919, -0.9988909959793091, -0.04690299928188324, 0.18705999851226807, -0.9821630120277405, -0.018818000331521034, 0.10363999754190445, -0.9898579716682434, 0.09715499728918076, 0.04757700115442276, -0.9953129887580872, 0.08418799936771393, -0.02296699956059456, -0.9986780285835266, -0.04599199816584587, -0.3861910104751587, -0.9210079908370972, -0.016421999782323837, -0.38136398792266846, -0.9230089783668518, 0.000155999994603917, -0.020431000739336014, -0.9987260103225708, 0.04614400118589401, -0.3750790059566498, -0.9061710238456726, -0.0007169999880716205, -0.19216600060462952, -0.9812189936637878, 0.01677200011909008, -0.3782620131969452, -0.9142940044403076, -0.00011300000187475234, -0.10471200197935104, -0.9897390007972717, -0.09722500294446945, -0.3655939996242523, -0.9257190227508545, 0.028463000431656837, -0.047710999846458435, -0.9953050017356873, -0.08420699834823608, -0.7092679738998413, -0.7031199932098389, -0.016364000737667084, -0.7061989903450012, -0.7061989903450012, 0, -0.6937360167503357, -0.6937360167503357, 0, -0.6997770071029663, -0.6997770071029663, 0, -0.6924030184745789, -0.7150859832763672, 0.02822900004684925, -0.9243540167808533, -0.37810400128364563, -0.01657800003886223, -0.9230089783668518, -0.38136398792266846, -0.000155999994603917, -0.9061710238456726, -0.3750790059566498, 0.0007169999880716205, -0.9142940044403076, -0.3782620131969452, 0.00011300000187475234, -0.9133660197257996, -0.39544400572776794, 0.028490999713540077, -0.998727023601532, -0.012897999957203865, 0.048757001757621765, -0.9987300038337708, -0.012866999953985214, -0.04870200157165527, -0.9812139868736267, -0.19210100173950195, -0.01775900088250637, -0.9897350072860718, -0.10451500117778778, 0.09747499972581863, -0.9953050017356873, -0.04767199978232384, 0.08423800021409988, -0.998727023601532, 0.012897999957203865, -0.048757001757621765, -0.9210079908370972, 0.3861910104751587, -0.016421999782323837, -0.9230089783668518, 0.38136398792266846, 0.000155999994603917, -0.9987300038337708, 0.012866999953985214, 0.04870200157165527, -0.9061710238456726, 0.3750790059566498, -0.0007169999880716205, -0.9812139868736267, 0.19210100173950195, 0.01775900088250637, -0.9142940044403076, 0.3782620131969452, -0.00011300000187475234, -0.9897350072860718, 0.10451500117778778, -0.09747499972581863, -0.9257190227508545, 0.3655939996242523, 0.028463000431656837, -0.9953050017356873, 0.04767199978232384, -0.08423800021409988, -0.7031199932098389, 0.7092679738998413, -0.016364000737667084, -0.7061989903450012, 0.7061989903450012, 0, -0.6937360167503357, 0.6937360167503357, 0, -0.6997770071029663, 0.6997770071029663, 0, -0.7150859832763672, 0.6924030184745789, 0.02822900004684925, -0.37810400128364563, 0.9243540167808533, -0.01657800003886223, -0.38136398792266846, 0.9230089783668518, -0.000155999994603917, -0.3750790059566498, 0.9061710238456726, 0.0007169999880716205, -0.3782620131969452, 0.9142940044403076, 0.00011300000187475234, -0.39544400572776794, 0.9133660197257996, 0.028490999713540077, -0.012897999957203865, 0.998727023601532, 0.048757001757621765, -0.012866999953985214, 0.9987300038337708, -0.04870200157165527, -0.19210100173950195, 0.9812139868736267, -0.01775900088250637, -0.10451500117778778, 0.9897350072860718, 0.09747499972581863, -0.04767199978232384, 0.9953050017356873, 0.08423800021409988, 0.04767199978232384, 0.9953050017356873, -0.08423800021409988, 0.39544400572776794, 0.9133660197257996, -0.028490999713540077, 0.38111698627471924, 0.9210190176963806, -0.000015999999959603883, 0.031922999769449234, 0.9968529939651489, -0.07255599647760391, 0.3815299868583679, 0.9219080209732056, 0.0000019999999949504854, 0.022261999547481537, 0.9978039860725403, -0.06237399950623512, 0.3821389973163605, 0.9231889843940735, 0.00001700000029813964, 0.008317999541759491, 0.9991790056228638, -0.03964800015091896, 0.38228899240493774, 0.9239469766616821, -0.004430000204592943, 0.0008660000166855752, 0.9999139904975891, 0.013048999942839146, 0.7150859832763672, 0.6924030184745789, -0.02822900004684925, 0.7048519849777222, 0.7048519849777222, 0, 0.7055330276489258, 0.7055330276489258, 0, 0.7065179944038391, 0.7065179944038391, 0, 0.7068390250205994, 0.707252025604248, -0.004379999823868275, 0.9257190227508545, 0.3655939996242523, -0.028463000431656837, 0.9210180044174194, 0.38111698627471924, 0.000015999999959603883, 0.9219080209732056, 0.3815299868583679, -0.0000019999999949504854, 0.9231889843940735, 0.3821389973163605, -0.00001700000029813964, 0.9237229824066162, 0.38283199071884155, -0.004399999976158142, 0.9953050017356873, 0.04767199978232384, 0.08423800021409988, 0.9968529939651489, 0.031922999769449234, 0.07255599647760391, 0.9978039860725403, 0.022261999547481537, 0.06237399950623512, 0.9991790056228638, 0.008317999541759491, 0.03964800015091896, 0.9999139904975891, 0.0008660000166855752, -0.013048999942839146, 0.9953050017356873, -0.04767199978232384, -0.08423800021409988, 0.9135000109672546, -0.3951619863510132, -0.02861100062727928, 0.9210190176963806, -0.38111698627471924, -0.000015999999959603883, 0.9968529939651489, -0.031922999769449234, -0.07255599647760391, 0.9219080209732056, -0.3815299868583679, 0.0000019999999949504854, 0.9978039860725403, -0.022261999547481537, -0.06237399950623512, 0.9231889843940735, -0.3821389973163605, 0.00001700000029813964, 0.9991790056228638, -0.008317999541759491, -0.03964800015091896, 0.9239469766616821, -0.38228899240493774, -0.004430000204592943, 0.9999139904975891, -0.0008660000166855752, 0.013048999942839146, 0.6925899982452393, -0.7149369716644287, -0.028262000530958176, 0.7048519849777222, -0.7048519849777222, 0, 0.7055330276489258, -0.7055330276489258, 0, 0.7065179944038391, -0.7065179944038391, 0, 0.707252025604248, -0.7068390250205994, -0.004379999823868275, 0.3656100034713745, -0.9257280230522156, -0.02841299958527088, 0.38111698627471924, -0.9210180044174194, 0.000015999999959603883, 0.3815299868583679, -0.9219080209732056, -0.0000019999999949504854, 0.3821389973163605, -0.9231889843940735, -0.00001700000029813964, 0.38283199071884155, -0.9237229824066162, -0.004399999976158142, 0.04757700115442276, -0.9953129887580872, 0.08418799936771393, 0.031922999769449234, -0.9968529939651489, 0.07255599647760391, 0.022261999547481537, -0.9978039860725403, 0.06237399950623512, 0.008317999541759491, -0.9991790056228638, 0.03964800015091896, 0.0008660000166855752, -0.9999139904975891, -0.013048999942839146, -0.047710999846458435, -0.9953050017356873, -0.08420699834823608, -0.39544400572776794, -0.9133660197257996, -0.028490999713540077, -0.38111698627471924, -0.9210190176963806, -0.000015999999959603883, -0.031922999769449234, -0.9968529939651489, -0.07255599647760391, -0.3815299868583679, -0.9219080209732056, 0.0000019999999949504854, -0.022261999547481537, -0.9978039860725403, -0.06237399950623512, -0.3821389973163605, -0.9231889843940735, 0.00001700000029813964, -0.008317999541759491, -0.9991790056228638, -0.03964800015091896, -0.38228899240493774, -0.9239469766616821, -0.004430000204592943, -0.0008660000166855752, -0.9999139904975891, 0.013048999942839146, -0.7150859832763672, -0.6924030184745789, -0.02822900004684925, -0.7048519849777222, -0.7048519849777222, 0, -0.7055330276489258, -0.7055330276489258, 0, -0.7065179944038391, -0.7065179944038391, 0, -0.7068390250205994, -0.707252025604248, -0.004379999823868275, -0.9257190227508545, -0.3655939996242523, -0.028463000431656837, -0.9210180044174194, -0.38111698627471924, 0.000015999999959603883, -0.9219080209732056, -0.3815299868583679, -0.0000019999999949504854, -0.9231889843940735, -0.3821389973163605, -0.00001700000029813964, -0.9237229824066162, -0.38283199071884155, -0.004399999976158142, -0.9953050017356873, -0.04767199978232384, 0.08423800021409988, -0.9968529939651489, -0.031922999769449234, 0.07255599647760391, -0.9978039860725403, -0.022261999547481537, 0.06237399950623512, -0.9991790056228638, -0.008317999541759491, 0.03964800015091896, -0.9999139904975891, -0.0008660000166855752, -0.013048999942839146, -0.9953050017356873, 0.04767199978232384, -0.08423800021409988, -0.9133660197257996, 0.39544400572776794, -0.028490999713540077, -0.9210190176963806, 0.38111698627471924, -0.000015999999959603883, -0.9968529939651489, 0.031922999769449234, -0.07255599647760391, -0.9219080209732056, 0.3815299868583679, 0.0000019999999949504854, -0.9978039860725403, 0.022261999547481537, -0.06237399950623512, -0.9231889843940735, 0.3821389973163605, 0.00001700000029813964, -0.9991790056228638, 0.008317999541759491, -0.03964800015091896, -0.9239469766616821, 0.38228899240493774, -0.004430000204592943, -0.9999139904975891, 0.0008660000166855752, 0.013048999942839146, -0.6924030184745789, 0.7150859832763672, -0.02822900004684925, -0.7048519849777222, 0.7048519849777222, 0, -0.7055330276489258, 0.7055330276489258, 0, -0.7065179944038391, 0.7065179944038391, 0, -0.707252025604248, 0.7068390250205994, -0.004379999823868275, -0.3655939996242523, 0.9257190227508545, -0.028463000431656837, -0.38111698627471924, 0.9210180044174194, 0.000015999999959603883, -0.3815299868583679, 0.9219080209732056, -0.0000019999999949504854, -0.3821389973163605, 0.9231889843940735, -0.00001700000029813964, -0.38283199071884155, 0.9237229824066162, -0.004399999976158142, -0.04767199978232384, 0.9953050017356873, 0.08423800021409988, -0.031922999769449234, 0.9968529939651489, 0.07255599647760391, -0.022261999547481537, 0.9978039860725403, 0.06237399950623512, -0.008317999541759491, 0.9991790056228638, 0.03964800015091896, -0.0008660000166855752, 0.9999139904975891, -0.013048999942839146, 0.0008660000166855752, 0.9999139904975891, 0.013048999942839146, 0.38283199071884155, 0.9237229824066162, 0.004399999976158142, 0.38101500272750854, 0.9204739928245544, -0.00003899999865097925, 0.03731299936771393, 0.9963229894638062, 0.07712399959564209, 0.37877199053764343, 0.9154880046844482, 0.00008399999933317304, 0.09151100367307663, 0.9910060167312622, 0.097632996737957, 0.378387987613678, 0.9145749807357788, 0.00009999999747378752, 0.10134600102901459, 0.9900450110435486, 0.09767600148916245, 0.356795996427536, 0.9266510009765625, -0.03188199922442436, 0.07246600091457367, 0.9928709864616394, 0.09463199973106384, 0.707252025604248, 0.7068390250205994, 0.004379999823868275, 0.7044739723205566, 0.7044739723205566, 0, 0.7006790041923523, 0.7006790041923523, 0, 0.6999930143356323, 0.6999930143356323, 0, 0.6847820281982422, 0.7192310094833374, -0.03167999908328056, 0.9239469766616821, 0.38228899240493774, 0.004430000204592943, 0.9204739928245544, 0.38101500272750854, 0.00003899999865097925, 0.9154880046844482, 0.37877199053764343, -0.00008399999933317304, 0.9145749807357788, 0.378387987613678, -0.00009999999747378752, 0.9078760147094727, 0.40216198563575745, -0.03206299990415573, 0.9999139904975891, 0.0008660000166855752, -0.013048999942839146, 0.9963229894638062, 0.03731299936771393, -0.07712399959564209, 0.9910060167312622, 0.09151100367307663, -0.097632996737957, 0.9900450110435486, 0.10134600102901459, -0.09767600148916245, 0.9928709864616394, 0.07246600091457367, -0.09463199973106384, 0.9999139904975891, -0.0008660000166855752, 0.013048999942839146, 0.9237229824066162, -0.38283199071884155, 0.004399999976158142, 0.9204739928245544, -0.38101500272750854, -0.00003899999865097925, 0.9963229894638062, -0.03731299936771393, 0.07712399959564209, 0.9154880046844482, -0.37877199053764343, 0.00008399999933317304, 0.9910060167312622, -0.09151100367307663, 0.097632996737957, 0.9145749807357788, -0.378387987613678, 0.00009999999747378752, 0.9900450110435486, -0.10134600102901459, 0.09767600148916245, 0.9266510009765625, -0.356795996427536, -0.03188199922442436, 0.9928709864616394, -0.07246600091457367, 0.09463199973106384, 0.7068390250205994, -0.707252025604248, 0.004379999823868275, 0.7044739723205566, -0.7044739723205566, 0, 0.7006790041923523, -0.7006790041923523, 0, 0.6999930143356323, -0.6999930143356323, 0, 0.7192310094833374, -0.6847820281982422, -0.03167999908328056, 0.38228899240493774, -0.9239469766616821, 0.004430000204592943, 0.38101500272750854, -0.9204739928245544, 0.00003899999865097925, 0.37877199053764343, -0.9154880046844482, -0.00008399999933317304, 0.378387987613678, -0.9145749807357788, -0.00009999999747378752, 0.40216198563575745, -0.9078760147094727, -0.03206299990415573, 0.0008660000166855752, -0.9999139904975891, -0.013048999942839146, 0.03731299936771393, -0.9963229894638062, -0.07712399959564209, 0.09151100367307663, -0.9910060167312622, -0.097632996737957, 0.10134600102901459, -0.9900450110435486, -0.09767600148916245, 0.07246600091457367, -0.9928709864616394, -0.09463199973106384, -0.0008660000166855752, -0.9999139904975891, 0.013048999942839146, -0.38283199071884155, -0.9237229824066162, 0.004399999976158142, -0.38101500272750854, -0.9204739928245544, -0.00003899999865097925, -0.03731299936771393, -0.9963229894638062, 0.07712399959564209, -0.37877199053764343, -0.9154880046844482, 0.00008399999933317304, -0.09151100367307663, -0.9910060167312622, 0.097632996737957, -0.378387987613678, -0.9145749807357788, 0.00009999999747378752, -0.10134600102901459, -0.9900450110435486, 0.09767600148916245, -0.356795996427536, -0.9266510009765625, -0.03188199922442436, -0.07246600091457367, -0.9928709864616394, 0.09463199973106384, -0.707252025604248, -0.7068390250205994, 0.004379999823868275, -0.7044739723205566, -0.7044739723205566, 0, -0.7006790041923523, -0.7006790041923523, 0, -0.6999930143356323, -0.6999930143356323, 0, -0.6847820281982422, -0.7192310094833374, -0.03167999908328056, -0.9239469766616821, -0.38228899240493774, 0.004430000204592943, -0.9204739928245544, -0.38101500272750854, 0.00003899999865097925, -0.9154880046844482, -0.37877199053764343, -0.00008399999933317304, -0.9145749807357788, -0.378387987613678, -0.00009999999747378752, -0.9078760147094727, -0.40216198563575745, -0.03206299990415573, -0.9999139904975891, -0.0008660000166855752, -0.013048999942839146, -0.9963229894638062, -0.03731299936771393, -0.07712399959564209, -0.9910060167312622, -0.09151100367307663, -0.097632996737957, -0.9900450110435486, -0.10134600102901459, -0.09767600148916245, -0.9928709864616394, -0.07246600091457367, -0.09463199973106384, -0.9999139904975891, 0.0008660000166855752, 0.013048999942839146, -0.9237229824066162, 0.38283199071884155, 0.004399999976158142, -0.9204739928245544, 0.38101500272750854, -0.00003899999865097925, -0.9963229894638062, 0.03731299936771393, 0.07712399959564209, -0.9154880046844482, 0.37877199053764343, 0.00008399999933317304, -0.9910060167312622, 0.09151100367307663, 0.097632996737957, -0.9145749807357788, 0.378387987613678, 0.00009999999747378752, -0.9900450110435486, 0.10134600102901459, 0.09767600148916245, -0.9266510009765625, 0.356795996427536, -0.03188199922442436, -0.9928709864616394, 0.07246600091457367, 0.09463199973106384, -0.7068390250205994, 0.707252025604248, 0.004379999823868275, -0.7044739723205566, 0.7044739723205566, 0, -0.7006790041923523, 0.7006790041923523, 0, -0.6999930143356323, 0.6999930143356323, 0, -0.7192310094833374, 0.6847820281982422, -0.03167999908328056, -0.38228899240493774, 0.9239469766616821, 0.004430000204592943, -0.38101500272750854, 0.9204739928245544, 0.00003899999865097925, -0.37877199053764343, 0.9154880046844482, -0.00008399999933317304, -0.378387987613678, 0.9145749807357788, -0.00009999999747378752, -0.40216198563575745, 0.9078760147094727, -0.03206299990415573, -0.0008660000166855752, 0.9999139904975891, -0.013048999942839146, -0.03731299936771393, 0.9963229894638062, -0.07712399959564209, -0.09151100367307663, 0.9910060167312622, -0.097632996737957, -0.10134600102901459, 0.9900450110435486, -0.09767600148916245, -0.07246600091457367, 0.9928709864616394, -0.09463199973106384, 0.07246600091457367, 0.9928709864616394, 0.09463199973106384, 0.40216198563575745, 0.9078760147094727, 0.03206299990415573, 0.37766799330711365, 0.912958025932312, 0.00018099999579135329, 0.11919300258159637, 0.9883019924163818, 0.09514500200748444, 0.37516000866889954, 0.906607985496521, 0.00016799999866634607, 0.187733992934227, 0.9816380143165588, 0.03381900116801262, 0.2823430001735687, 0.767549991607666, -0.1682250052690506, 0.12883399426937103, 0.6540690064430237, -0.32698601484298706, 0.06457000225782394, 0.32701900601387024, -0.6666669845581055, 0, 0, -1, 0.7192320227622986, 0.6847820281982422, 0.03167999908328056, 0.6987630128860474, 0.6987630128860474, 0, 0.694034993648529, 0.694034993648529, 0, 0.5551990270614624, 0.6008960008621216, -0.16825300455093384, 0.1854030042886734, 0.27701398730278015, -0.6666669845581055, 0.9266499876976013, 0.3567950129508972, 0.03188199922442436, 0.912958025932312, 0.37766799330711365, -0.00018099999579135329, 0.906607985496521, 0.37516000866889954, -0.00016799999866634607, 0.742605984210968, 0.3426159918308258, -0.1683180034160614, 0.27701398730278015, 0.1854030042886734, -0.6666669845581055, 0.9928709864616394, 0.07246600091457367, -0.09463199973106384, 0.9883019924163818, 0.11919300258159637, -0.09514500200748444, 0.9816370010375977, 0.187733992934227, -0.03381900116801262, 0.9811030030250549, 0.19325199723243713, -0.009519999846816063, 0.49052900075912476, 0.0968559980392456, -0.5, 0.9928709864616394, -0.07246600091457367, 0.09463199973106384, 0.9078760147094727, -0.40216198563575745, 0.03206299990415573, 0.912958025932312, -0.37766799330711365, 0.00018099999579135329, 0.9883019924163818, -0.11919300258159637, 0.09514500200748444, 0.906607985496521, -0.37516000866889954, 0.00016799999866634607, 0.9816380143165588, -0.187733992934227, 0.03381900116801262, 0.767549991607666, -0.2823430001735687, -0.1682250052690506, 0.6540690064430237, -0.12883399426937103, -0.32698601484298706, 0.32701900601387024, -0.06457000225782394, -0.6666669845581055, 0, 0, -1, 0.6847820281982422, -0.7192320227622986, 0.03167999908328056, 0.6987630128860474, -0.6987630128860474, 0, 0.694034993648529, -0.694034993648529, 0, 0.6008960008621216, -0.5551990270614624, -0.16825300455093384, 0.27701398730278015, -0.1854030042886734, -0.6666669845581055, 0.3567950129508972, -0.9266499876976013, 0.03188199922442436, 0.37766799330711365, -0.912958025932312, -0.00018099999579135329, 0.37516000866889954, -0.906607985496521, -0.00016799999866634607, 0.3426159918308258, -0.742605984210968, -0.1683180034160614, 0.1854030042886734, -0.27701398730278015, -0.6666669845581055, 0.07246600091457367, -0.9928709864616394, -0.09463199973106384, 0.11919300258159637, -0.9883019924163818, -0.09514500200748444, 0.187733992934227, -0.9816370010375977, -0.03381900116801262, 0.19325199723243713, -0.9811030030250549, -0.009519999846816063, 0.0968559980392456, -0.49052900075912476, -0.5, -0.07246600091457367, -0.9928709864616394, 0.09463199973106384, -0.40216198563575745, -0.9078760147094727, 0.03206299990415573, -0.37766799330711365, -0.912958025932312, 0.00018099999579135329, -0.11919300258159637, -0.9883019924163818, 0.09514500200748444, -0.37516000866889954, -0.906607985496521, 0.00016799999866634607, -0.187733992934227, -0.9816380143165588, 0.03381900116801262, -0.2823430001735687, -0.767549991607666, -0.1682250052690506, -0.12883399426937103, -0.6540690064430237, -0.32698601484298706, -0.06457000225782394, -0.32701900601387024, -0.6666669845581055, 0, 0, -1, -0.7192320227622986, -0.6847820281982422, 0.03167999908328056, -0.6987630128860474, -0.6987630128860474, 0, -0.694034993648529, -0.694034993648529, 0, -0.5551990270614624, -0.6008960008621216, -0.16825300455093384, -0.1854030042886734, -0.27701398730278015, -0.6666669845581055, -0.9266499876976013, -0.3567950129508972, 0.03188199922442436, -0.912958025932312, -0.37766799330711365, -0.00018099999579135329, -0.906607985496521, -0.37516000866889954, -0.00016799999866634607, -0.742605984210968, -0.3426159918308258, -0.1683180034160614, -0.27701398730278015, -0.1854030042886734, -0.6666669845581055, -0.9928709864616394, -0.07246600091457367, -0.09463199973106384, -0.9883019924163818, -0.11919300258159637, -0.09514500200748444, -0.9816370010375977, -0.187733992934227, -0.03381900116801262, -0.9811030030250549, -0.19325199723243713, -0.009519999846816063, -0.49052900075912476, -0.0968559980392456, -0.5, -0.9928709864616394, 0.07246600091457367, 0.09463199973106384, -0.9078760147094727, 0.40216198563575745, 0.03206299990415573, -0.912958025932312, 0.37766799330711365, 0.00018099999579135329, -0.9883019924163818, 0.11919300258159637, 0.09514500200748444, -0.906607985496521, 0.37516000866889954, 0.00016799999866634607, -0.9816380143165588, 0.187733992934227, 0.03381900116801262, -0.767549991607666, 0.2823430001735687, -0.1682250052690506, -0.6540690064430237, 0.12883399426937103, -0.32698601484298706, -0.32701900601387024, 0.06457000225782394, -0.6666669845581055, 0, 0, -1, -0.6847820281982422, 0.7192320227622986, 0.03167999908328056, -0.6987630128860474, 0.6987630128860474, 0, -0.694034993648529, 0.694034993648529, 0, -0.6008960008621216, 0.5551990270614624, -0.16825300455093384, -0.27701398730278015, 0.1854030042886734, -0.6666669845581055, -0.3567950129508972, 0.9266499876976013, 0.03188199922442436, -0.37766799330711365, 0.912958025932312, -0.00018099999579135329, -0.37516000866889954, 0.906607985496521, -0.00016799999866634607, -0.3426159918308258, 0.742605984210968, -0.1683180034160614, -0.1854030042886734, 0.27701398730278015, -0.6666669845581055, -0.07246600091457367, 0.9928709864616394, -0.09463199973106384, -0.11919300258159637, 0.9883019924163818, -0.09514500200748444, -0.187733992934227, 0.9816370010375977, -0.03381900116801262, -0.19325199723243713, 0.9811030030250549, -0.009519999846816063, -0.0968559980392456, 0.49052900075912476, -0.5, -0.006597999949008226, 0.9961680173873901, 0.0001630000042496249, -0.043907999992370605, 0.779125988483429, -0.55936598777771, 0.23287899792194366, 0.79271000623703, -0.506534993648529, 0.11139900237321854, 0.9923329949378967, 0.0053449999541044235, 0.4521920084953308, 0.7370989918708801, -0.42180201411247253, 0.17797799408435822, 0.9827970266342163, 0.036841001361608505, 0.6075379848480225, 0.7066869735717773, -0.270797997713089, 0.11894699931144714, 0.9864829778671265, 0.10517799854278564, 0.6583719849586487, 0.7438470125198364, -0.06727500259876251, 0.0010629999451339245, 0.99891597032547, 0.04653400182723999, -0.1622990071773529, -0.14869500696659088, -0.9069569706916809, 0.3020159900188446, -0.014301000162959099, -0.8847119808197021, 0.7048640251159668, -0.042514998465776443, -0.6788020133972168, 0.8948519825935364, -0.11078000068664551, -0.38824599981307983, 0.9622920155525208, -0.09367900341749191, -0.14349600672721863, -0.12511900067329407, -0.8479049801826477, -0.4783349931240082, 0.11315400153398514, -0.8153669834136963, -0.5167160034179688, 0.3956319987773895, -0.7910019755363464, -0.4345270097255707, 0.5244609713554382, -0.8012329936027527, -0.2643829882144928, 0.571465015411377, -0.7902160286903381, -0.12332800030708313, -0.0943560004234314, -0.9955379962921143, -0.0010989999864250422, 0.012040999718010426, -0.9965500235557556, 0, 0.09501499682664871, -0.9936969876289368, 0.02440500073134899, 0.03737499937415123, -0.9978089928627014, 0.035909999161958694, -0.0008800000068731606, -0.9973530173301697, -0.04031199961900711, 0.007164000067859888, -0.9961649775505066, -0.00002700000004551839, 0.043988000601530075, -0.8330309987068176, 0.4691329896450043, -0.2334270030260086, -0.7983189821243286, 0.49840399622917175, -0.10737399756908417, -0.9927549958229065, -0.007029999978840351, -0.45147499442100525, -0.7576299905776978, 0.39375001192092896, -0.15364399552345276, -0.9863160252571106, -0.048294998705387115, -0.5575600266456604, -0.7753210067749023, 0.2001740038394928, -0.07242999970912933, -0.9923030138015747, -0.08845999836921692, -0.5877019762992859, -0.8041930198669434, 0.04768599942326546, 0.0005830000154674053, -0.9997940063476562, -0.020301999524235725, 0.13663700222969055, -0.14665700495243073, 0.8966140151023865, -0.3045389950275421, -0.012237999588251114, 0.8833180069923401, -0.7020289897918701, -0.033987998962402344, 0.6724730134010315, -0.8890330195426941, -0.09636799991130829, 0.37605398893356323, -0.9668099880218506, -0.08601800352334976, 0.1358419954776764, 0.12022499740123749, 0.7918559908866882, 0.5693140029907227, -0.11313500255346298, 0.8111780285835266, 0.5236610174179077, -0.39790698885917664, 0.7734419703483582, 0.45853298902511597, -0.5793390274047852, 0.7346490025520325, 0.32973799109458923, -0.6447499990463257, 0.7340419888496399, 0.12459299713373184, 0.09378799796104431, 0.9955919981002808, 0.000944000028539449, -0.01607999950647354, 0.9964879751205444, 0.00035600000410340726, -0.11933200061321259, 0.9912199974060059, -0.01737299934029579, -0.08618299663066864, 0.9940080046653748, -0.053598999977111816, -0.004110999871045351, 0.9980229735374451, 0.015703000128269196, 0.010142000392079353, 0.9933879971504211, 0.10034400224685669, 0.6597890257835388, 0.7114480137825012, 0.12964099645614624, 0.5634239912033081, 0.7594000101089478, 0.289902001619339, -0.021227000281214714, 0.9976930022239685, 0.05189099907875061, 0.3972559869289398, 0.7709670066833496, 0.45872700214385986, -0.05054600164294243, 0.9957669973373413, 0.060869000852108, 0.11805199831724167, 0.7611619830131531, 0.5692800283432007, -0.11414600163698196, 0.9869359731674194, 0.08862999826669693, -0.0012870000209659338, 0.7195389866828918, 0.6293820142745972, -0.18971200287342072, 0.9752820134162903, 0.11328700184822083, 0.9685969948768616, -0.08966200053691864, 0.13331100344657898, 0.8902140259742737, -0.051961999386548996, 0.39323100447654724, 0.6728280186653137, -0.050324998795986176, 0.6965069770812988, 0.25133201479911804, -0.04306900128722191, 0.9169719815254211, -0.19813700020313263, -0.2512879967689514, 0.9046909809112549, 0.5937719941139221, -0.8024669885635376, 0.03307799994945526, 0.5571249723434448, -0.7907459735870361, 0.2022089958190918, 0.4313510060310364, -0.8083119988441467, 0.37996000051498413, 0.19395600259304047, -0.8197799921035767, 0.5133119821548462, -0.1517219990491867, -0.8084930181503296, 0.5055829882621765, 0.0035200000274926424, -0.9997940063476562, 0.019979000091552734, 0.01159599982202053, -0.9981369972229004, -0.02326199971139431, 0.01310999970883131, -0.9988970160484314, -0.008480999618768692, -0.02485400065779686, -0.9978809952735901, 0.021263999864459038, -0.11335399746894836, -0.9881970286369324, 0.06441199779510498, -0.0035459999926388264, -0.9954169988632202, -0.07682599872350693, -0.5816869735717773, -0.7760900259017944, -0.13957500457763672, -0.5260769724845886, -0.790789008140564, -0.2781960070133209, 0.017288999632000923, -0.9983699917793274, -0.03728000074625015, -0.36800798773765564, -0.7982890009880066, -0.4405499994754791, 0.03743100166320801, -0.9973520040512085, -0.03640099987387657, -0.09636899828910828, -0.7829139828681946, -0.5500450134277344, 0.10426300019025803, -0.9894949793815613, -0.06746900081634521, 0.10083399713039398, -0.8161320090293884, -0.48112401366233826, 0.18510299921035767, -0.9776470065116882, -0.09971100091934204, -0.9615049958229065, -0.08203399926424026, -0.14958199858665466, -0.8876789808273315, -0.04622500017285347, -0.39955899119377136, -0.6675580143928528, -0.03723999857902527, -0.7007560133934021, -0.245511993765831, -0.03216199949383736, -0.9151920080184937, 0.15477199852466583, -0.24929499626159668, -0.8975690007209778, -0.6700729727745056, 0.7402250170707703, -0.01942499913275242, -0.5923460125923157, 0.7624830007553101, -0.21566900610923767, -0.45611900091171265, 0.7868310213088989, -0.39906400442123413, -0.21001900732517242, 0.8031420111656189, -0.5333020091056824, 0.05119999870657921, 0.7096909880638123, -0.6591699719429016, -0.014175999909639359, 0.9989240169525146, -0.04416000097990036, -0.0065449997782707214, 0.9983869791030884, 0.008813999593257904, 0.0023960000835359097, 0.9989259839057922, -0.016711000353097916, 0.03813000023365021, 0.9969249963760376, -0.04171599820256233, 0.11744900047779083, 0.986670970916748, -0.0799890011548996, -0.02072799950838089, -0.997963011264801, 0.0017740000039339066, 0.10236400365829468, -0.695684015750885, -0.6961740255355835, 0.28174999356269836, -0.7065439820289612, -0.6379269957542419, -0.027713999152183533, -0.9983959794044495, -0.016395000740885735, 0.4621469974517822, -0.7501789927482605, -0.43765199184417725, -0.014942999929189682, -0.9960020184516907, -0.04751100018620491, 0.6121799945831299, -0.7355859875679016, -0.1658719927072525, 0.08200599998235703, -0.9833409786224365, 0.11102399975061417, 0.7232419848442078, -0.6012910008430481, -0.14595800638198853, 0.32238098978996277, -0.9036369919776917, 0.28197699785232544, 0.1188960000872612, 0.09661199897527695, -0.9692260026931763, 0.3230240046977997, 0.06791900098323822, -0.9069269895553589, 0.6287810206413269, 0.00962899997830391, -0.711097002029419, 0.8952469825744629, -0.060169998556375504, -0.3366979956626892, 0.9689210057258606, -0.04508800059556961, -0.13095800578594208, 0.06500200182199478, 0.7708680033683777, -0.6083509922027588, 0.1816529929637909, 0.7457069754600525, -0.593995988368988, 0.37600401043891907, 0.7467949986457825, -0.4776870012283325, 0.6288849711418152, 0.7020969986915588, -0.27160701155662537, 0.8230010271072388, 0.5295370221138, -0.09450399875640869, -0.12820099294185638, 0.9899809956550598, -0.05917999893426895, -0.11097600311040878, 0.9872509837150574, -0.09937400370836258, -0.06767299771308899, 0.9865689873695374, -0.1427209973335266, -0.0003349999897181988, 0.9967420101165771, 0.025443999096751213, 0.29019099473953247, 0.9243509769439697, 0.1957239955663681, 0.07294999808073044, 0.9949049949645996, 0.03147900104522705, -0.04948300123214722, 0.7695090174674988, 0.6163870096206665, -0.24193400144577026, 0.7750219702720642, 0.5679330229759216, 0.05620399862527847, 0.9959489703178406, 0.052143000066280365, -0.4294399917125702, 0.779321014881134, 0.41615501046180725, 0.023887999355793, 0.9943940043449402, 0.07553800195455551, -0.6655910015106201, 0.6939520239830017, 0.20106400549411774, -0.09678799659013748, 0.9791589975357056, -0.12869000434875488, -0.7716730237007141, 0.5443729758262634, 0.1793539971113205, -0.417836993932724, 0.8721759915351868, -0.2544029951095581, -0.09499499946832657, 0.08934500068426132, 0.9787889719009399, -0.3299880027770996, 0.06701900064945221, 0.9273520112037659, -0.6511250138282776, 0.023523999378085136, 0.7280719876289368, -0.9116759896278381, -0.033263999968767166, 0.34162598848342896, -0.9896330237388611, -0.013496000319719315, 0.07834099978208542, -0.07044100016355515, -0.6954740285873413, 0.7080140113830566, -0.21969600021839142, -0.6959800124168396, 0.6642320156097412, -0.4075010120868683, -0.7370589971542358, 0.5047789812088013, -0.5866039991378784, -0.7473030090332031, 0.24636299908161163, -0.799036979675293, -0.5617390275001526, 0.05794600024819374, 0.07605399936437607, -0.9967970252037048, 0.02472200058400631, 0.08756300061941147, -0.9926980137825012, 0.05929899960756302, 0.07250799983739853, -0.9901790022850037, 0.11122000217437744, 0.015556000173091888, -0.9970260262489319, -0.011235999874770641, -0.194814994931221, -0.9439409971237183, -0.22127500176429749, 0.3417310118675232, -0.8896859884262085, 0.3012309968471527, 0.8375009894371033, -0.4931910037994385, 0.05739299952983856, 0.8273029923439026, -0.4684619903564453, -0.05539099872112274, 0.5311300158500671, -0.8121910095214844, 0.24026300013065338, 0.8069959878921509, -0.47689300775527954, 0.002638000063598156, 0.644743025302887, -0.7642210125923157, -0.015455000102519989, 0.8856800198554993, -0.4464530050754547, 0.047488000243902206, -0.011536000296473503, -0.999845027923584, -0.0008730000117793679, 0.7597830295562744, -0.6229599714279175, 0.026636000722646713, 0.321245014667511, -0.8855000138282776, 0.3356960117816925, 0.998091995716095, -0.005673000123351812, 0.025262000039219856, 0.9941530227661133, 0.046904999762773514, -0.00951599981635809, 0.9838590025901794, -0.00041700000292621553, 0.010572000406682491, 0.990556001663208, 0.01886500045657158, 0.04422200098633766, 0.9921990036964417, -0.12290599942207336, 0.011202000081539154, 0.828000009059906, 0.5258169770240784, -0.0846100002527237, 0.8704839944839478, 0.4878079891204834, 0.00635599996894598, 0.7773939967155457, 0.5659670233726501, -0.09634699672460556, 0.8190580010414124, 0.4740380048751831, 0.01190400030463934, 0.9017590284347534, 0.3486430048942566, -0.05601400136947632, 0.41038599610328674, 0.870602011680603, 0.27135801315307617, 0.3019320070743561, 0.8897680044174194, 0.34101900458335876, 0.13912299275398254, 0.9423390030860901, -0.3042120039463043, 0.6167309880256653, 0.7692840099334717, 0.1667650043964386, 0.5558350086212158, 0.8010749816894531, 0.21867799758911133, -0.4410029947757721, 0.8555399775505066, -0.2693159878253937, -0.8639690279960632, 0.464356005191803, -0.019222000613808632, -0.8705710172653198, 0.4855479896068573, -0.005623999983072281, -0.33969300985336304, 0.8762779831886292, -0.34097298979759216, -0.7608209848403931, 0.5840269923210144, 0.11236599832773209, -0.16763299703598022, 0.9419429898262024, 0.29091599583625793, -0.8260639905929565, 0.47304999828338623, -0.0134699996560812, -0.6006280183792114, 0.7822970151901245, -0.1611420065164566, -0.8495870232582092, 0.4440779983997345, 0.17417700588703156, -0.5251449942588806, 0.8236340284347534, -0.21412399411201477, -0.9991480112075806, 0.0017519999528303742, 0.007890000008046627, -0.9946579933166504, 0.06129400059580803, 0.007796999998390675, -0.9840919971466064, 0.008732999674975872, -0.0001289999927394092, -0.9916059970855713, 0.015207000076770782, -0.04798699915409088, -0.9899899959564209, -0.13816699385643005, -0.019433999434113503, -0.7927820086479187, -0.5669599771499634, 0.06795799732208252, -0.8363490104675293, -0.4685719907283783, 0.048955000936985016, -0.8138830065727234, -0.4743089973926544, 0.0008379999781027436, -0.8869869709014893, -0.4417180120944977, -0.05625399947166443, -0.7898640036582947, -0.5522750020027161, -0.15016800165176392, -0.297340989112854, -0.8998129963874817, -0.3192580044269562, -0.49759799242019653, -0.8317790031433105, -0.24411599338054657, -0.6295620203018188, -0.7765420079231262, 0.01261799968779087, -0.011338000185787678, -0.9998990297317505, -0.008561000227928162, -0.3547320067882538, -0.8679590225219727, -0.3453510105609894, 0.09618999809026718, 0.49066001176834106, -0.5, 0.1851000040769577, 0.27721700072288513, -0.6666669845581055, 0.32566601037979126, 0.76139897108078, -0.18199099600315094, 0.062401000410318375, 0.9939020276069641, -0.09090700000524521, 0.3803209960460663, 0.9214360117912292, -0.00007100000220816582, 0.030918000265955925, 0.9969729781150818, 0.07133600115776062, 0.3804109990596771, 0.9220889806747437, 0.0001630000042496249, 0.02471200004220009, 0.9975799918174744, 0.06498300284147263, 0.35510900616645813, 0.926891028881073, 0.03216100111603737, 0.07657899707555771, 0.9924740195274353, -0.09555599838495255, 0.27721700072288513, 0.1851000040769577, -0.6666669845581055, 0.5929989814758301, 0.5781109929084778, -0.18205299973487854, 0.7048519849777222, 0.7048519849777222, 0, 0.7052720189094543, 0.7054179906845093, -0.00002499999936844688, 0.6835219860076904, 0.7199410200119019, 0.03204600140452385, 0.3271070122718811, 0.06412599980831146, -0.6666669845581055, 0.7694699764251709, 0.3061000108718872, -0.18225300312042236, 0.9214379787445068, 0.38033199310302734, 0.0000670000008540228, 0.9220880270004272, 0.3804430067539215, -0.00016799999866634607, 0.9071130156517029, 0.403003990650177, 0.032437000423669815, 0, 0, -1, 0.6626030206680298, 0.04157499969005585, -0.272724986076355, 0.9969789981842041, 0.03082600049674511, -0.07129299640655518, 0.9975910186767578, 0.024447999894618988, -0.06492199748754501, 0.9925040006637573, 0.07630900293588638, 0.09545700252056122, 0.49066001176834106, -0.09618999809026718, -0.5, 0.27721700072288513, -0.1851000040769577, -0.6666669845581055, 0.76139897108078, -0.32566601037979126, -0.18199099600315094, 0.9939020276069641, -0.062401000410318375, -0.09090700000524521, 0.9214360117912292, -0.3803209960460663, -0.00007100000220816582, 0.9969729781150818, -0.030918000265955925, 0.07133600115776062, 0.9220889806747437, -0.3804109990596771, 0.0001630000042496249, 0.9975799918174744, -0.02471200004220009, 0.06498300284147263, 0.926891028881073, -0.35510900616645813, 0.03216100111603737, 0.9924740195274353, -0.07657899707555771, -0.09555599838495255, 0.1851000040769577, -0.27721700072288513, -0.6666669845581055, 0.5781109929084778, -0.5929989814758301, -0.18205299973487854, 0.7048519849777222, -0.7048519849777222, 0, 0.7054179906845093, -0.7052720189094543, -0.00002499999936844688, 0.7199410200119019, -0.6835219860076904, 0.03204600140452385, 0.06412599980831146, -0.3271070122718811, -0.6666669845581055, 0.3061000108718872, -0.7694699764251709, -0.18225300312042236, 0.38033199310302734, -0.9214379787445068, 0.0000670000008540228, 0.3804430067539215, -0.9220880270004272, -0.00016799999866634607, 0.403003990650177, -0.9071130156517029, 0.032437000423669815, 0, 0, -1, 0.04157499969005585, -0.6626030206680298, -0.272724986076355, 0.03082600049674511, -0.9969789981842041, -0.07129299640655518, 0.024447999894618988, -0.9975910186767578, -0.06492199748754501, 0.07630900293588638, -0.9925040006637573, 0.09545700252056122, -0.09618999809026718, -0.49066001176834106, -0.5, -0.1851000040769577, -0.27721700072288513, -0.6666669845581055, -0.32566601037979126, -0.76139897108078, -0.18199099600315094, -0.062401000410318375, -0.9939020276069641, -0.09090700000524521, -0.3803209960460663, -0.9214360117912292, -0.00007100000220816582, -0.030918000265955925, -0.9969729781150818, 0.07133600115776062, -0.3804109990596771, -0.9220889806747437, 0.0001630000042496249, -0.02471200004220009, -0.9975799918174744, 0.06498300284147263, -0.35510900616645813, -0.926891028881073, 0.03216100111603737, -0.07657899707555771, -0.9924740195274353, -0.09555599838495255, -0.27721700072288513, -0.1851000040769577, -0.6666669845581055, -0.5929989814758301, -0.5781109929084778, -0.18205299973487854, -0.7048519849777222, -0.7048519849777222, 0, -0.7052720189094543, -0.7054179906845093, -0.00002499999936844688, -0.6835219860076904, -0.7199410200119019, 0.03204600140452385, -0.3271070122718811, -0.06412599980831146, -0.6666669845581055, -0.7694699764251709, -0.3061000108718872, -0.18225300312042236, -0.9214379787445068, -0.38033199310302734, 0.0000670000008540228, -0.9220880270004272, -0.3804430067539215, -0.00016799999866634607, -0.9071130156517029, -0.403003990650177, 0.032437000423669815, 0, 0, -1, -0.6626030206680298, -0.04157499969005585, -0.272724986076355, -0.9969789981842041, -0.03082600049674511, -0.07129299640655518, -0.9975910186767578, -0.024447999894618988, -0.06492199748754501, -0.9925040006637573, -0.07630900293588638, 0.09545700252056122, -0.49066001176834106, 0.09618999809026718, -0.5, -0.27721700072288513, 0.1851000040769577, -0.6666669845581055, -0.76139897108078, 0.32566601037979126, -0.18199099600315094, -0.9939020276069641, 0.062401000410318375, -0.09090700000524521, -0.9214360117912292, 0.3803209960460663, -0.00007100000220816582, -0.9969729781150818, 0.030918000265955925, 0.07133600115776062, -0.9220889806747437, 0.3804109990596771, 0.0001630000042496249, -0.9975799918174744, 0.02471200004220009, 0.06498300284147263, -0.926891028881073, 0.35510900616645813, 0.03216100111603737, -0.9924740195274353, 0.07657899707555771, -0.09555599838495255, -0.1851000040769577, 0.27721700072288513, -0.6666669845581055, -0.5781109929084778, 0.5929989814758301, -0.18205299973487854, -0.7048519849777222, 0.7048519849777222, 0, -0.7054179906845093, 0.7052720189094543, -0.00002499999936844688, -0.7199410200119019, 0.6835219860076904, 0.03204600140452385, -0.06412599980831146, 0.3271070122718811, -0.6666669845581055, -0.3061000108718872, 0.7694699764251709, -0.18225300312042236, -0.38033199310302734, 0.9214379787445068, 0.0000670000008540228, -0.3804430067539215, 0.9220880270004272, -0.00016799999866634607, -0.403003990650177, 0.9071130156517029, 0.032437000423669815, 0, 0, -1, -0.04157499969005585, 0.6626030206680298, -0.272724986076355, -0.03082600049674511, 0.9969789981842041, -0.07129299640655518, -0.024447999894618988, 0.9975910186767578, -0.06492199748754501, -0.07630900293588638, 0.9925040006637573, 0.09545700252056122, 0.07657899707555771, 0.9924740195274353, -0.09555599838495255, 0.40307098627090454, 0.9070649743080139, -0.03255299851298332, 0.3753640055656433, 0.9070209860801697, 0.000007000000096013537, 0.18306200206279755, 0.9820899963378906, -0.04457399994134903, 0.3751649856567383, 0.9065750241279602, -0.00007400000322377309, 0.18801499903202057, 0.9816100001335144, -0.03304100036621094, 0.3759070038795471, 0.908607006072998, -0.00026199998683296144, 0.16623400151729584, 0.983722984790802, -0.06822899729013443, 0.33324098587036133, 0.9290030002593994, 0.029803000390529633, 0.14071400463581085, 0.9862040281295776, -0.08718100190162659, 0.7198299765586853, 0.6836559772491455, -0.032017000019550323, 0.6943539977073669, 0.6943539977073669, 0, 0.694034993648529, 0.694034993648529, 0, 0.6955100297927856, 0.6955100297927856, 0, 0.6639170050621033, 0.7306150197982788, 0.029100999236106873, 0.9268649816513062, 0.35523301362991333, -0.03203999996185303, 0.9070209860801697, 0.3753649890422821, -0.000007000000096013537, 0.9065750241279602, 0.3751649856567383, 0.00007300000288523734, 0.908607006072998, 0.3759070038795471, 0.00026199998683296144, 0.8926259875297546, 0.4211460053920746, 0.028991999104619026, 0.9924740195274353, 0.07646500319242477, 0.09565100073814392, 0.9820899963378906, 0.18306200206279755, 0.04457399994134903, 0.9816100001335144, 0.18801499903202057, 0.03304100036621094, 0.983722984790802, 0.16623400151729584, 0.06822899729013443, 0.9862040281295776, 0.14071400463581085, 0.08718100190162659, 0.9924740195274353, -0.07657899707555771, -0.09555599838495255, 0.9070649743080139, -0.40307098627090454, -0.03255299851298332, 0.9070209860801697, -0.3753640055656433, 0.000007000000096013537, 0.9820899963378906, -0.18306200206279755, -0.04457399994134903, 0.9065750241279602, -0.3751649856567383, -0.00007400000322377309, 0.9816100001335144, -0.18801499903202057, -0.03304100036621094, 0.908607006072998, -0.3759070038795471, -0.00026199998683296144, 0.983722984790802, -0.16623400151729584, -0.06822899729013443, 0.9290030002593994, -0.33324098587036133, 0.029803000390529633, 0.9862040281295776, -0.14071400463581085, -0.08718100190162659, 0.6836559772491455, -0.7198299765586853, -0.032017000019550323, 0.6943539977073669, -0.6943539977073669, 0, 0.694034993648529, -0.694034993648529, 0, 0.6955100297927856, -0.6955100297927856, 0, 0.7306150197982788, -0.6639170050621033, 0.029100999236106873, 0.35523301362991333, -0.9268649816513062, -0.03203999996185303, 0.3753649890422821, -0.9070209860801697, -0.000007000000096013537, 0.3751649856567383, -0.9065750241279602, 0.00007300000288523734, 0.3759070038795471, -0.908607006072998, 0.00026199998683296144, 0.4211460053920746, -0.8926259875297546, 0.028991999104619026, 0.07646500319242477, -0.9924740195274353, 0.09565100073814392, 0.18306200206279755, -0.9820899963378906, 0.04457399994134903, 0.18801499903202057, -0.9816100001335144, 0.03304100036621094, 0.16623400151729584, -0.983722984790802, 0.06822899729013443, 0.14071400463581085, -0.9862040281295776, 0.08718100190162659, -0.07657899707555771, -0.9924740195274353, -0.09555599838495255, -0.40307098627090454, -0.9070649743080139, -0.03255299851298332, -0.3753640055656433, -0.9070209860801697, 0.000007000000096013537, -0.18306200206279755, -0.9820899963378906, -0.04457399994134903, -0.3751649856567383, -0.9065750241279602, -0.00007400000322377309, -0.18801499903202057, -0.9816100001335144, -0.03304100036621094, -0.3759070038795471, -0.908607006072998, -0.00026199998683296144, -0.16623400151729584, -0.983722984790802, -0.06822899729013443, -0.33324098587036133, -0.9290030002593994, 0.029803000390529633, -0.14071400463581085, -0.9862040281295776, -0.08718100190162659, -0.7198299765586853, -0.6836559772491455, -0.032017000019550323, -0.6943539977073669, -0.6943539977073669, 0, -0.694034993648529, -0.694034993648529, 0, -0.6955100297927856, -0.6955100297927856, 0, -0.6639170050621033, -0.7306150197982788, 0.029100999236106873, -0.9268649816513062, -0.35523301362991333, -0.03203999996185303, -0.9070209860801697, -0.3753649890422821, -0.000007000000096013537, -0.9065750241279602, -0.3751649856567383, 0.00007300000288523734, -0.908607006072998, -0.3759070038795471, 0.00026199998683296144, -0.8926259875297546, -0.4211460053920746, 0.028991999104619026, -0.9924740195274353, -0.07646500319242477, 0.09565100073814392, -0.9820899963378906, -0.18306200206279755, 0.04457399994134903, -0.9816100001335144, -0.18801499903202057, 0.03304100036621094, -0.983722984790802, -0.16623400151729584, 0.06822899729013443, -0.9862040281295776, -0.14071400463581085, 0.08718100190162659, -0.9924740195274353, 0.07657899707555771, -0.09555599838495255, -0.9070649743080139, 0.40307098627090454, -0.03255299851298332, -0.9070209860801697, 0.3753640055656433, 0.000007000000096013537, -0.9820899963378906, 0.18306200206279755, -0.04457399994134903, -0.9065750241279602, 0.3751649856567383, -0.00007400000322377309, -0.9816100001335144, 0.18801499903202057, -0.03304100036621094, -0.908607006072998, 0.3759070038795471, -0.00026199998683296144, -0.983722984790802, 0.16623400151729584, -0.06822899729013443, -0.9290030002593994, 0.33324098587036133, 0.029803000390529633, -0.9862040281295776, 0.14071400463581085, -0.08718100190162659, -0.6836559772491455, 0.7198299765586853, -0.032017000019550323, -0.6943539977073669, 0.6943539977073669, 0, -0.694034993648529, 0.694034993648529, 0, -0.6955100297927856, 0.6955100297927856, 0, -0.7306150197982788, 0.6639170050621033, 0.029100999236106873, -0.35523301362991333, 0.9268649816513062, -0.03203999996185303, -0.3753649890422821, 0.9070209860801697, -0.000007000000096013537, -0.3751649856567383, 0.9065750241279602, 0.00007300000288523734, -0.3759070038795471, 0.908607006072998, 0.00026199998683296144, -0.4211460053920746, 0.8926259875297546, 0.028991999104619026, -0.07646500319242477, 0.9924740195274353, 0.09565100073814392, -0.18306200206279755, 0.9820899963378906, 0.04457399994134903, -0.18801499903202057, 0.9816100001335144, 0.03304100036621094, -0.16623400151729584, 0.983722984790802, 0.06822899729013443, -0.14071400463581085, 0.9862040281295776, 0.08718100190162659
  };
  core->addAttributeData(OGLConstants::TANGENT.location, tangents,
      sizeof(tangents), 3, GL_STATIC_DRAW);

  // define binormals
  const GLfloat binormals[] = {
      0.2554270029067993, -0.05043400079011917, -0.9655119776725769, 0.2302899956703186, -0.11379700154066086, -0.9664459824562073, -0.23653900623321533, 0.09789499640464783, -0.9666780233383179, -0.2551180124282837, 0.05037299916148186, -0.9655969738960266, -0.9201610088348389, 0.38079801201820374, -0.09108299762010574, -0.9770479798316956, 0.1929199993610382, -0.09032399952411652, -0.6762400269508362, 0.2798590064048767, 0.6814529895782471, -0.723800003528595, 0.1429159939289093, 0.6750479936599731, -0.4681990146636963, 0.1581760048866272, 0.869350016117096, -0.4902079999446869, 0.09679199755191803, 0.8662149906158447, 0.16952399909496307, -0.1934960037469864, -0.9663439989089966, -0.18106800317764282, 0.18106800317764282, -0.9666590094566345, -0.7041199803352356, 0.7041199803352356, -0.09181900322437286, -0.5179349780082703, 0.5179349780082703, 0.6807990074157715, -0.37217798829078674, 0.3260670006275177, 0.8690019845962524, 0.08221600204706192, -0.243368998169899, -0.9664430022239685, -0.09789499640464783, 0.23653900623321533, -0.9666780233383179, -0.38079801201820374, 0.9201610088348389, -0.09108199924230576, -0.2798590064048767, 0.6762400269508362, 0.6814540028572083, -0.21894000470638275, 0.44305500388145447, 0.8693490028381348, 0.050822000950574875, -0.2573910057544708, -0.9649699926376343, -0.05021600052714348, 0.25432100892066956, -0.965815007686615, -0.19291600584983826, 0.9770249724388123, -0.09059000015258789, -0.14291299879550934, 0.7237870097160339, 0.6750609874725342, -0.09679199755191803, 0.4902079999446869, 0.8662149906158447, -0.048507001250982285, -0.2576940059661865, -0.965008020401001, -0.15833300352096558, -0.3227809965610504, -0.933135986328125, 0.05656199902296066, 0.13793900609016418, -0.9888240098953247, 0.049150001257658005, 0.2545199990272522, -0.9658179879188538, 0.378387987613678, 0.9173290133476257, -0.12381099909543991, 0.1917950063943863, 0.9772530198097229, -0.09050799906253815, 0.2777239978313446, 0.6716070175170898, 0.6868870258331299, 0.14281700551509857, 0.7238019704818726, 0.6750659942626953, 0.15788200497627258, 0.4674209952354431, 0.8698220252990723, 0.09679199755191803, 0.4902079999446869, 0.8662149906158447, -0.3139069974422455, -0.2657270133495331, -0.9115110039710999, 0.05247500166296959, 0.05178600177168846, -0.9972789883613586, 0.699787974357605, 0.6969379782676697, -0.15676100552082062, 0.511929988861084, 0.5116159915924072, 0.6900550127029419, 0.32515400648117065, 0.37111398577690125, 0.8697980046272278, -0.3181929886341095, -0.09987600147724152, -0.9427499771118164, 0.1552799940109253, 0.06176299974322319, -0.9859380125999451, 0.9187250137329102, 0.3751460015773773, -0.1233299970626831, 0.6724870204925537, 0.2775439918041229, 0.6860979795455933, 0.4424299895763397, 0.21853800117969513, 0.8697689771652222, -0.255948007106781, -0.04464200139045715, -0.9656590223312378, 0.25306200981140137, 0.046362001448869705, -0.9663389921188354, 0.9778940081596375, 0.18800100684165955, -0.09153299778699875, 0.7238150238990784, 0.14205799996852875, 0.675212025642395, 0.49017900228500366, 0.0967010036110878, 0.8662409782409668, -0.25491899251937866, 0.05033399909734726, -0.9656509757041931, -0.2302899956703186, 0.11379700154066086, -0.9664459824562073, 0.23653900623321533, -0.09789499640464783, -0.9666780233383179, 0.252265989780426, -0.04980999976396561, -0.9663749933242798, 0.9201610088348389, -0.38079801201820374, -0.09108299762010574, 0.9769039750099182, -0.19289200007915497, -0.09193000197410583, 0.6762400269508362, -0.2798590064048767, 0.6814529895782471, 0.7236610054969788, -0.14288799464702606, 0.6752020120620728, 0.4681990146636963, -0.1581760048866272, 0.869350016117096, 0.4901660084724426, -0.09678400307893753, 0.8662390112876892, -0.16952399909496307, 0.1934960037469864, -0.9663439989089966, 0.18106800317764282, -0.18106800317764282, -0.9666590094566345, 0.7041199803352356, -0.7041199803352356, -0.09181900322437286, 0.5179349780082703, -0.5179349780082703, 0.6807990074157715, 0.37217798829078674, -0.3260670006275177, 0.8690019845962524, -0.08221600204706192, 0.243368998169899, -0.9664430022239685, 0.09789499640464783, -0.23653900623321533, -0.9666780233383179, 0.38079801201820374, -0.9201610088348389, -0.09108199924230576, 0.2798590064048767, -0.6762400269508362, 0.6814540028572083, 0.21894000470638275, -0.44305500388145447, 0.8693490028381348, -0.05043400079011917, 0.2554270029067993, -0.9655119776725769, 0.05037299916148186, -0.2551180124282837, -0.9655969738960266, 0.1929199993610382, -0.9770479798316956, -0.09032399952411652, 0.1429159939289093, -0.723800003528595, 0.6750479936599731, 0.09679199755191803, -0.4902079999446869, 0.8662149906158447, 0.05043400079011917, 0.2554270029067993, -0.9655119776725769, 0.11379700154066086, 0.2302899956703186, -0.9664459824562073, -0.09789499640464783, -0.23653900623321533, -0.9666780233383179, -0.05037299916148186, -0.2551180124282837, -0.9655969738960266, -0.38079801201820374, -0.9201610088348389, -0.09108299762010574, -0.1929199993610382, -0.9770479798316956, -0.09032399952411652, -0.2798590064048767, -0.6762400269508362, 0.6814529895782471, -0.1429159939289093, -0.723800003528595, 0.6750479936599731, -0.1581760048866272, -0.4681990146636963, 0.869350016117096, -0.09679199755191803, -0.4902079999446869, 0.8662149906158447, 0.1934960037469864, 0.16952399909496307, -0.9663439989089966, -0.18106800317764282, -0.18106800317764282, -0.9666590094566345, -0.7041199803352356, -0.7041199803352356, -0.09181900322437286, -0.5179349780082703, -0.5179349780082703, 0.6807990074157715, -0.3260670006275177, -0.37217798829078674, 0.8690019845962524, 0.243368998169899, 0.08221600204706192, -0.9664430022239685, -0.23653900623321533, -0.09789499640464783, -0.9666780233383179, -0.9201610088348389, -0.38079801201820374, -0.09108199924230576, -0.6762400269508362, -0.2798590064048767, 0.6814540028572083, -0.44305500388145447, -0.21894000470638275, 0.8693490028381348, 0.2554270029067993, 0.05043400079011917, -0.9655119776725769, -0.2551180124282837, -0.05037299916148186, -0.9655969738960266, -0.9770479798316956, -0.1929199993610382, -0.09032399952411652, -0.723800003528595, -0.1429159939289093, 0.6750479936599731, -0.4902079999446869, -0.09679199755191803, 0.8662149906158447, -0.4902079999446869, 0.09679199755191803, 0.8662149906158447, -0.44305500388145447, 0.21893900632858276, 0.8693490028381348, -0.37287598848342896, 0.15431199967861176, 0.9149600267410278, -0.4014579951763153, 0.07926800101995468, 0.9124410152435303, -0.3112579882144928, 0.12881100177764893, 0.9415550231933594, -0.33541300892829895, 0.0662280023097992, 0.939740002155304, -0.19015200436115265, 0.07869099825620651, 0.9785959720611572, -0.20517399907112122, 0.040511999279260635, 0.977886974811554, 0.06301800161600113, -0.021289000287652016, 0.9977849721908569, 0.06623400002717972, -0.013078000396490097, 0.9977179765701294, -0.3260670006275177, 0.37217798829078674, 0.8690019845962524, -0.285739004611969, 0.285739004611969, 0.9147170186042786, -0.23854400217533112, 0.23854400217533112, 0.9413790106773376, -0.14574900269508362, 0.14574900269508362, 0.9785270094871521, 0.05011200159788132, -0.04390300065279007, 0.9977779984474182, -0.1581760048866272, 0.4681999981403351, 0.869350016117096, -0.15431199967861176, 0.37287598848342896, 0.9149600267410278, -0.12881100177764893, 0.3112579882144928, 0.9415550231933594, -0.07869099825620651, 0.19015100598335266, 0.9785959720611572, 0.02946699969470501, -0.05963199958205223, 0.9977849721908569, -0.09679199755191803, 0.4902079999446869, 0.8662149906158447, -0.07926800101995468, 0.4014579951763153, 0.9124410152435303, -0.0662280023097992, 0.33541300892829895, 0.939740002155304, -0.040511999279260635, 0.20517399907112122, 0.977886974811554, 0.013078000396490097, -0.06623400002717972, 0.9977179765701294, 0.09679199755191803, 0.4902079999446869, 0.8662149906158447, 0.21858200430870056, 0.4423219859600067, 0.86981201171875, 0.15431199967861176, 0.37287598848342896, 0.9149600267410278, 0.07926800101995468, 0.4014579951763153, 0.9124410152435303, 0.12881100177764893, 0.3112579882144928, 0.9415550231933594, 0.0662280023097992, 0.33541300892829895, 0.939740002155304, 0.07869099825620651, 0.19015200436115265, 0.9785959720611572, 0.040511999279260635, 0.20517399907112122, 0.977886974811554, -0.021289000287652016, -0.06301800161600113, 0.9977849721908569, -0.013078000396490097, -0.06623400002717972, 0.9977179765701294, 0.3711329996585846, 0.3251489996910095, 0.8697919845581055, 0.285739004611969, 0.285739004611969, 0.9147170186042786, 0.23854400217533112, 0.23854400217533112, 0.9413790106773376, 0.14574900269508362, 0.14574900269508362, 0.9785270094871521, -0.04390300065279007, -0.05011200159788132, 0.9977779984474182, 0.46750199794769287, 0.15794099867343903, 0.8697680234909058, 0.37287598848342896, 0.15431199967861176, 0.9149600267410278, 0.3112579882144928, 0.12881100177764893, 0.9415550231933594, 0.19015100598335266, 0.07869099825620651, 0.9785959720611572, -0.05963199958205223, -0.02946699969470501, 0.9977849721908569, 0.49017900228500366, 0.0967010036110878, 0.8662409782409668, 0.4014579951763153, 0.07926800101995468, 0.9124410152435303, 0.33541300892829895, 0.0662280023097992, 0.939740002155304, 0.20517399907112122, 0.040511999279260635, 0.977886974811554, -0.06623400002717972, -0.013078000396490097, 0.9977179765701294, 0.4901660084724426, -0.09678400307893753, 0.8662390112876892, 0.44305500388145447, -0.21893900632858276, 0.8693490028381348, 0.37287598848342896, -0.15431199967861176, 0.9149600267410278, 0.4014579951763153, -0.07926800101995468, 0.9124410152435303, 0.3112579882144928, -0.12881100177764893, 0.9415550231933594, 0.33541300892829895, -0.0662280023097992, 0.939740002155304, 0.19015200436115265, -0.07869099825620651, 0.9785959720611572, 0.20517399907112122, -0.040511999279260635, 0.977886974811554, -0.06301800161600113, 0.021289000287652016, 0.9977849721908569, -0.06623400002717972, 0.013078000396490097, 0.9977179765701294, 0.3260670006275177, -0.37217798829078674, 0.8690019845962524, 0.285739004611969, -0.285739004611969, 0.9147170186042786, 0.23854400217533112, -0.23854400217533112, 0.9413790106773376, 0.14574900269508362, -0.14574900269508362, 0.9785270094871521, -0.05011200159788132, 0.04390300065279007, 0.9977779984474182, 0.1581760048866272, -0.4681999981403351, 0.869350016117096, 0.15431199967861176, -0.37287598848342896, 0.9149600267410278, 0.12881100177764893, -0.3112579882144928, 0.9415550231933594, 0.07869099825620651, -0.19015100598335266, 0.9785959720611572, -0.02946699969470501, 0.05963199958205223, 0.9977849721908569, 0.09679199755191803, -0.4902079999446869, 0.8662149906158447, 0.07926800101995468, -0.4014579951763153, 0.9124410152435303, 0.0662280023097992, -0.33541300892829895, 0.939740002155304, 0.040511999279260635, -0.20517399907112122, 0.977886974811554, -0.013078000396490097, 0.06623400002717972, 0.9977179765701294, -0.09679199755191803, -0.4902079999446869, 0.8662149906158447, -0.21893900632858276, -0.44305500388145447, 0.8693490028381348, -0.15431199967861176, -0.37287598848342896, 0.9149600267410278, -0.07926800101995468, -0.4014579951763153, 0.9124410152435303, -0.12881100177764893, -0.3112579882144928, 0.9415550231933594, -0.0662280023097992, -0.33541300892829895, 0.939740002155304, -0.07869099825620651, -0.19015200436115265, 0.9785959720611572, -0.040511999279260635, -0.20517399907112122, 0.977886974811554, 0.021289000287652016, 0.06301800161600113, 0.9977849721908569, 0.013078000396490097, 0.06623400002717972, 0.9977179765701294, -0.37217798829078674, -0.3260670006275177, 0.8690019845962524, -0.285739004611969, -0.285739004611969, 0.9147170186042786, -0.23854400217533112, -0.23854400217533112, 0.9413790106773376, -0.14574900269508362, -0.14574900269508362, 0.9785270094871521, 0.04390300065279007, 0.05011200159788132, 0.9977779984474182, -0.4681999981403351, -0.1581760048866272, 0.869350016117096, -0.37287598848342896, -0.15431199967861176, 0.9149600267410278, -0.3112579882144928, -0.12881100177764893, 0.9415550231933594, -0.19015100598335266, -0.07869099825620651, 0.9785959720611572, 0.05963199958205223, 0.02946699969470501, 0.9977849721908569, -0.4902079999446869, -0.09679199755191803, 0.8662149906158447, -0.4014579951763153, -0.07926800101995468, 0.9124410152435303, -0.33541300892829895, -0.0662280023097992, 0.939740002155304, -0.20517399907112122, -0.040511999279260635, 0.977886974811554, 0.06623400002717972, 0.013078000396490097, 0.9977179765701294, 0.06623400002717972, -0.013078000396490097, 0.9977179765701294, 0.05963199958205223, -0.02946699969470501, 0.9977849721908569, 0.40303200483322144, -0.1667889952659607, 0.8998590111732483, 0.4339120090007782, -0.08567699790000916, 0.8968719840049744, 0.6326310038566589, -0.2618109881877899, 0.7288579940795898, 0.6777120232582092, -0.13381600379943848, 0.723048985004425, 0.6661339998245239, -0.27567601203918457, 0.6930140256881714, 0.7128540277481079, -0.14075499773025513, 0.6870430111885071, 0.5777599811553955, -0.19519099593162537, 0.792523980140686, 0.6036490201950073, -0.11919199675321579, 0.7882900238037109, 0.04390300065279007, -0.05011200159788132, 0.9977779984474182, 0.30884799361228943, -0.30884799361228943, 0.8995699882507324, 0.48457300662994385, -0.48457300662994385, 0.7282710075378418, 0.510138988494873, -0.510138988494873, 0.6924710273742676, 0.4591110050678253, -0.40222999453544617, 0.7921029925346375, 0.021289000287652016, -0.06301800161600113, 0.9977849721908569, 0.16678999364376068, -0.40303200483322144, 0.8998590111732483, 0.2618109881877899, -0.6326310038566589, 0.7288579940795898, 0.27567601203918457, -0.6661339998245239, 0.6930140256881714, 0.2701770067214966, -0.546737015247345, 0.7925170063972473, 0.013078000396490097, -0.06623400002717972, 0.9977179765701294, 0.08567599952220917, -0.4339120090007782, 0.8968719840049744, 0.13381600379943848, -0.6777120232582092, 0.723048985004425, 0.14075499773025513, -0.7128540277481079, 0.6870430111885071, 0.11919199675321579, -0.6036490201950073, 0.7882900238037109, -0.013078000396490097, -0.06623400002717972, 0.9977179765701294, -0.02946699969470501, -0.05963199958205223, 0.9977849721908569, -0.1667889952659607, -0.40303200483322144, 0.8998590111732483, -0.08567699790000916, -0.4339120090007782, 0.8968719840049744, -0.2618109881877899, -0.6326310038566589, 0.7288579940795898, -0.13381600379943848, -0.6777120232582092, 0.723048985004425, -0.27567601203918457, -0.6661339998245239, 0.6930140256881714, -0.14075499773025513, -0.7128540277481079, 0.6870430111885071, -0.19519099593162537, -0.5777599811553955, 0.792523980140686, -0.11919199675321579, -0.6036490201950073, 0.7882900238037109, -0.05011200159788132, -0.04390300065279007, 0.9977779984474182, -0.30884799361228943, -0.30884799361228943, 0.8995699882507324, -0.48457300662994385, -0.48457300662994385, 0.7282710075378418, -0.510138988494873, -0.510138988494873, 0.6924710273742676, -0.40222999453544617, -0.4591110050678253, 0.7921029925346375, -0.06301800161600113, -0.021289000287652016, 0.9977849721908569, -0.40303200483322144, -0.16678999364376068, 0.8998590111732483, -0.6326310038566589, -0.2618109881877899, 0.7288579940795898, -0.6661339998245239, -0.27567601203918457, 0.6930140256881714, -0.546737015247345, -0.2701770067214966, 0.7925170063972473, -0.06623400002717972, -0.013078000396490097, 0.9977179765701294, -0.4339120090007782, -0.08567599952220917, 0.8968719840049744, -0.6777120232582092, -0.13381600379943848, 0.723048985004425, -0.7128540277481079, -0.14075499773025513, 0.6870430111885071, -0.6036490201950073, -0.11919199675321579, 0.7882900238037109, -0.06623400002717972, 0.013078000396490097, 0.9977179765701294, -0.05963199958205223, 0.02946699969470501, 0.9977849721908569, -0.40303200483322144, 0.1667889952659607, 0.8998590111732483, -0.4339120090007782, 0.08567699790000916, 0.8968719840049744, -0.6326310038566589, 0.2618109881877899, 0.7288579940795898, -0.6777120232582092, 0.13381600379943848, 0.723048985004425, -0.6661339998245239, 0.27567601203918457, 0.6930140256881714, -0.7128540277481079, 0.14075499773025513, 0.6870430111885071, -0.5777599811553955, 0.19519099593162537, 0.792523980140686, -0.6036490201950073, 0.11919199675321579, 0.7882900238037109, -0.04390300065279007, 0.05011200159788132, 0.9977779984474182, -0.30884799361228943, 0.30884799361228943, 0.8995699882507324, -0.48457300662994385, 0.48457300662994385, 0.7282710075378418, -0.510138988494873, 0.510138988494873, 0.6924710273742676, -0.4591110050678253, 0.40222999453544617, 0.7921029925346375, -0.021289000287652016, 0.06301800161600113, 0.9977849721908569, -0.16678999364376068, 0.40303200483322144, 0.8998590111732483, -0.2618109881877899, 0.6326310038566589, 0.7288579940795898, -0.27567601203918457, 0.6661339998245239, 0.6930140256881714, -0.2701770067214966, 0.546737015247345, 0.7925170063972473, -0.013078000396490097, 0.06623400002717972, 0.9977179765701294, -0.08567599952220917, 0.4339120090007782, 0.8968719840049744, -0.13381600379943848, 0.6777120232582092, 0.723048985004425, -0.14075499773025513, 0.7128540277481079, 0.6870430111885071, -0.11919199675321579, 0.6036490201950073, 0.7882900238037109, 0.013078000396490097, 0.06623400002717972, 0.9977179765701294, 0.02946699969470501, 0.05963199958205223, 0.9977849721908569, 0.1667889952659607, 0.40303200483322144, 0.8998590111732483, 0.08567699790000916, 0.4339120090007782, 0.8968719840049744, 0.2618109881877899, 0.6326310038566589, 0.7288579940795898, 0.13381600379943848, 0.6777120232582092, 0.723048985004425, 0.27567601203918457, 0.6661339998245239, 0.6930140256881714, 0.14075499773025513, 0.7128540277481079, 0.6870430111885071, 0.19519099593162537, 0.5777599811553955, 0.792523980140686, 0.11919199675321579, 0.6036490201950073, 0.7882900238037109, 0.05011200159788132, 0.04390300065279007, 0.9977779984474182, 0.30884799361228943, 0.30884799361228943, 0.8995699882507324, 0.48457300662994385, 0.48457300662994385, 0.7282710075378418, 0.510138988494873, 0.510138988494873, 0.6924710273742676, 0.40222999453544617, 0.4591110050678253, 0.7921029925346375, 0.06301800161600113, 0.021289000287652016, 0.9977849721908569, 0.40303200483322144, 0.16678999364376068, 0.8998590111732483, 0.6326310038566589, 0.2618109881877899, 0.7288579940795898, 0.6661339998245239, 0.27567601203918457, 0.6930140256881714, 0.546737015247345, 0.2701770067214966, 0.7925170063972473, 0.06623400002717972, 0.013078000396490097, 0.9977179765701294, 0.4339120090007782, 0.08567599952220917, 0.8968719840049744, 0.6777120232582092, 0.13381600379943848, 0.723048985004425, 0.7128540277481079, 0.14075499773025513, 0.6870430111885071, 0.6036490201950073, 0.11919199675321579, 0.7882900238037109, 0.6036490201950073, -0.11919199675321579, 0.7882900238037109, 0.546737015247345, -0.2701770067214966, 0.7925170063972473, 0.7223830223083496, -0.2989569902420044, 0.623528003692627, 0.7723940014839172, -0.15251100063323975, 0.616562008857727, 0.9094089865684509, -0.3763520121574402, 0.1770150065422058, 0.9660869836807251, -0.19075599312782288, 0.1740349978208542, 0.9408230185508728, -0.3353259861469269, 0.04907499998807907, 0.9843119978904724, -0.16964000463485718, 0.048493001610040665, 0.9810580015182495, -0.1937119960784912, 0, 0.7071070075035095, -0.7071070075035095, 0, 0.40222999453544617, -0.4591110050678253, 0.7921029925346375, 0.5532029867172241, -0.5532029867172241, 0.622842013835907, 0.6959879994392395, -0.6959879994392395, 0.17663900554180145, 0.7403979897499084, -0.6703829765319824, 0.048958998173475266, 0.8310419917106628, -0.5562090277671814, 0, 0.19519099593162537, -0.5777599811553955, 0.792523980140686, 0.2989560067653656, -0.7223830223083496, 0.623528003692627, 0.3763520121574402, -0.9094089865684509, 0.1770150065422058, 0.4275760054588318, -0.902646005153656, 0.04906899854540825, 0.5562090277671814, -0.8310419917106628, 0, 0.11919199675321579, -0.6036490201950073, 0.7882900238037109, 0.15251100063323975, -0.7723940014839172, 0.616562008857727, 0.19075599312782288, -0.9660869836807251, 0.1740349978208542, 0.19348600506782532, -0.9799140095710754, 0.048277001827955246, 0.1937119960784912, -0.9810580015182495, 0, -0.11919199675321579, -0.6036490201950073, 0.7882900238037109, -0.2701770067214966, -0.546737015247345, 0.7925170063972473, -0.2989569902420044, -0.7223830223083496, 0.623528003692627, -0.15251100063323975, -0.7723940014839172, 0.616562008857727, -0.3763520121574402, -0.9094089865684509, 0.1770150065422058, -0.19075599312782288, -0.9660869836807251, 0.1740349978208542, -0.3353259861469269, -0.9408230185508728, 0.04907499998807907, -0.16964000463485718, -0.9843119978904724, 0.04849399998784065, -0.1937119960784912, -0.9810580015182495, 0, 0.7071070075035095, -0.7071070075035095, 0, -0.4591110050678253, -0.40222999453544617, 0.7921029925346375, -0.5532029867172241, -0.5532029867172241, 0.622842013835907, -0.6959879994392395, -0.6959879994392395, 0.17663900554180145, -0.6703829765319824, -0.7403979897499084, 0.048958998173475266, -0.5562090277671814, -0.8310419917106628, 0, -0.5777599811553955, -0.19519099593162537, 0.792523980140686, -0.7223830223083496, -0.2989560067653656, 0.623528003692627, -0.9094089865684509, -0.3763520121574402, 0.1770150065422058, -0.902646005153656, -0.4275760054588318, 0.04906899854540825, -0.8310419917106628, -0.5562090277671814, 0, -0.6036490201950073, -0.11919199675321579, 0.7882900238037109, -0.7723940014839172, -0.15251100063323975, 0.616562008857727, -0.9660869836807251, -0.19075599312782288, 0.1740349978208542, -0.9799140095710754, -0.19348600506782532, 0.048277001827955246, -0.9810580015182495, -0.1937119960784912, 0, -0.6036490201950073, 0.11919199675321579, 0.7882900238037109, -0.546737015247345, 0.2701770067214966, 0.7925170063972473, -0.7223830223083496, 0.2989569902420044, 0.623528003692627, -0.7723940014839172, 0.15251100063323975, 0.616562008857727, -0.9094089865684509, 0.3763520121574402, 0.1770150065422058, -0.9660869836807251, 0.19075599312782288, 0.1740349978208542, -0.9408230185508728, 0.3353259861469269, 0.04907499998807907, -0.9843119978904724, 0.16964000463485718, 0.04849399998784065, -0.9810580015182495, 0.1937119960784912, 0, 0.7071070075035095, -0.7071070075035095, 0, -0.40222999453544617, 0.4591110050678253, 0.7921029925346375, -0.5532029867172241, 0.5532029867172241, 0.622842013835907, -0.6959879994392395, 0.6959879994392395, 0.17663900554180145, -0.7403979897499084, 0.6703829765319824, 0.048958998173475266, -0.8310419917106628, 0.5562090277671814, 0, -0.19519099593162537, 0.5777599811553955, 0.792523980140686, -0.2989560067653656, 0.7223830223083496, 0.623528003692627, -0.3763520121574402, 0.9094089865684509, 0.1770150065422058, -0.4275760054588318, 0.902646005153656, 0.04906899854540825, -0.5562090277671814, 0.8310419917106628, 0, -0.11919199675321579, 0.6036490201950073, 0.7882900238037109, -0.15251100063323975, 0.7723940014839172, 0.616562008857727, -0.19075599312782288, 0.9660869836807251, 0.1740349978208542, -0.19348600506782532, 0.9799140095710754, 0.048277001827955246, -0.1937119960784912, 0.9810580015182495, 0, 0.11919199675321579, 0.6036490201950073, 0.7882900238037109, 0.2701770067214966, 0.546737015247345, 0.7925170063972473, 0.2989569902420044, 0.7223830223083496, 0.623528003692627, 0.15251100063323975, 0.7723940014839172, 0.616562008857727, 0.3763520121574402, 0.9094089865684509, 0.1770150065422058, 0.19075599312782288, 0.9660869836807251, 0.1740349978208542, 0.3353259861469269, 0.9408230185508728, 0.04907499998807907, 0.16964000463485718, 0.9843119978904724, 0.04849399998784065, 0.1937119960784912, 0.9810580015182495, 0, 0.7071070075035095, -0.7071070075035095, 0, 0.4591110050678253, 0.40222999453544617, 0.7921029925346375, 0.5532029867172241, 0.5532029867172241, 0.622842013835907, 0.6959879994392395, 0.6959879994392395, 0.17663900554180145, 0.6703829765319824, 0.7403979897499084, 0.048958998173475266, 0.5562090277671814, 0.8310419917106628, 0, 0.5777599811553955, 0.19519099593162537, 0.792523980140686, 0.7223830223083496, 0.2989560067653656, 0.623528003692627, 0.9094089865684509, 0.3763520121574402, 0.1770150065422058, 0.902646005153656, 0.4275760054588318, 0.04906899854540825, 0.8310419917106628, 0.5562090277671814, 0, 0.6036490201950073, 0.11919199675321579, 0.7882900238037109, 0.7723940014839172, 0.15251100063323975, 0.616562008857727, 0.9660869836807251, 0.19075599312782288, 0.1740349978208542, 0.9799150228500366, 0.19348600506782532, 0.048277001827955246, 0.9810580015182495, 0.1937119960784912, 0, 0.9999480247497559, 0.006622000131756067, 0.007786999922245741, 0.9989290237426758, 0.04125700145959854, -0.020945999771356583, 0.9700260162353516, -0.18230900168418884, 0.1606609970331192, 0.9929869771003723, -0.1116809993982315, 0.038782998919487, 0.8677089810371399, -0.30993399024009705, 0.38861599564552307, 0.9675049781799316, -0.18179599940776825, 0.17574100196361542, 0.6127229928970337, -0.23797500133514404, 0.753616988658905, 0.781611979007721, -0.1585649996995926, 0.6032750010490417, 0.13049399852752686, -0.02585900016129017, 0.9911119937896729, 0.16583800315856934, -0.04606600105762482, 0.9850770235061646, 0.9847609996795654, -0.03004699945449829, -0.17129500210285187, 0.9463850259780884, 0.008138000033795834, 0.3229379951953888, 0.6942890286445618, 0.06011800095438957, 0.7171810269355774, 0.405923992395401, 0.09244199842214584, 0.9092199802398682, 0.1477230042219162, 0.0024739999789744616, 0.9890260100364685, 0.991798996925354, -0.11557900160551071, -0.05454900115728378, 0.9920099973678589, 0.07202500104904175, 0.10358300060033798, 0.8882240056991577, 0.2238840013742447, 0.40116599202156067, 0.6046879887580872, 0.13691100478172302, 0.7846069931983948, 0.12908099591732025, -0.06111999973654747, 0.989749014377594, 0.9954820275306702, -0.09436299651861191, 0.010502999648451805, 0.9981970191001892, 0.012060999870300293, 0.05880200117826462, 0.9550639986991882, 0.09819000214338303, 0.27966299653053284, 0.6606940031051636, 0.05169999971985817, 0.7488729953765869, 0.07273799926042557, -0.04034300148487091, 0.9965350031852722, 0.9999179840087891, 0.007191000040620565, 0.010560999624431133, 0.9989050030708313, 0.04436499997973442, -0.014883999712765217, 0.9694769978523254, -0.17860299348831177, 0.1679760068655014, 0.9924619793891907, -0.10775599628686905, 0.058378998190164566, 0.8567489981651306, -0.28829601407051086, 0.4276289939880371, 0.9473999738693237, -0.16112199425697327, 0.27653801441192627, 0.5627779960632324, -0.19747799634933472, 0.8026729822158813, 0.6577669978141785, -0.11438000202178955, 0.7444859743118286, 0.07901199907064438, 0.0013689999468624592, 0.9968730211257935, 0.07274100184440613, -0.02020600065588951, 0.9971460103988647, 0.9888780117034912, 0.025808999314904213, -0.14647500216960907, 0.9453979730606079, -0.006663000211119652, 0.3258500099182129, 0.6921399831771851, -0.04972299933433533, 0.7200480103492737, 0.3957499861717224, -0.08134900033473969, 0.9147480130195618, 0.13914500176906586, -0.00007899999764049426, 0.9902719855308533, 0.9924669861793518, -0.10311000049114227, -0.06616800278425217, 0.9926300048828125, 0.07926999777555466, 0.09165900200605392, 0.900858998298645, 0.2553130090236664, 0.3510949909687042, 0.6513699889183044, 0.18318000435829163, 0.7363160252571106, 0.15978699922561646, -0.02714099921286106, 0.9867780208587646, 0.9955620169639587, -0.09379199892282486, 0.0077309999614953995, 0.9991030097007751, 0.01610800065100193, 0.039149001240730286, 0.9764699935913086, 0.12068899720907211, 0.17871999740600586, 0.7859060168266296, 0.10103499889373779, 0.6100350022315979, 0.16579000651836395, -0.014832000248134136, 0.986050009727478, 0.1655299961566925, -0.10078699886798859, 0.9810410141944885, -0.0046790000051259995, -0.1750659942626953, 0.9845460057258606, -0.3859579861164093, -0.06494200229644775, 0.9202280044555664, -0.3219670057296753, -0.05600599944591522, 0.9450929760932922, -0.6471610069274902, -0.11495299637317657, 0.7536370158195496, -0.5616440176963806, -0.078855000436306, 0.8236119747161865, -0.8379700183868408, -0.23749999701976776, 0.4913240075111389, -0.7512590289115906, -0.1447169929742813, 0.6439470052719116, -0.9052090048789978, -0.2807050049304962, 0.3190630078315735, -0.8249419927597046, -0.2209009975194931, 0.5202630162239075, -0.13363699615001678, 0.0291920006275177, 0.9905999898910522, -0.4039649963378906, 0.0019519999623298645, 0.9147719740867615, -0.7191359996795654, 0.002443999983370304, 0.6948649883270264, -0.9637579917907715, 0.026884999126195908, 0.26541900634765625, -0.9637719988822937, 0.2207069993019104, -0.14977200329303741, 0.03522900119423866, 0.06716900318861008, 0.9971190094947815, -0.3620629906654358, -0.01676199957728386, 0.9320030212402344, -0.6534259915351868, 0.007120999973267317, 0.7569569945335388, -0.8528590202331543, 0.11686599999666214, 0.5088940262794495, -0.8814889788627625, 0.3579840064048767, 0.3079349994659424, 0.0726580023765564, 0.02018200047314167, 0.9971529841423035, -0.37608298659324646, -0.025955000892281532, 0.926222026348114, -0.6568350195884705, -0.015021000057458878, 0.7538840174674988, -0.8238760232925415, 0.03257700055837631, 0.5658339858055115, -0.8688690066337585, 0.13078700006008148, 0.4774540066719055, 0.07265599817037582, -0.07700400054454803, 0.994379997253418, -0.0343950018286705, -0.151870995759964, 0.9878020286560059, -0.40362000465393066, -0.05282000079751015, 0.9134010076522827, -0.37586501240730286, -0.041078001260757446, 0.9257640242576599, -0.6878190040588379, -0.0810059979557991, 0.721347987651825, -0.6558970212936401, -0.052101001143455505, 0.7530509829521179, -0.8708800077438354, -0.2062380015850067, 0.44613200426101685, -0.8175939917564392, -0.12448199838399887, 0.5621780157089233, -0.8926960229873657, -0.3055669963359833, 0.33124300837516785, -0.8565059900283813, -0.21024300158023834, 0.4713769853115082, -0.15155400335788727, -0.025412000715732574, 0.9881219863891602, -0.41033700108528137, -0.0026420000940561295, 0.9119300246238708, -0.7240620255470276, 0.00047400000039488077, 0.6897349953651428, -0.9655590057373047, -0.017078999429941177, 0.2596229910850525, -0.973825991153717, -0.19711799919605255, -0.1131730005145073, 0.06214199960231781, 0.08235500007867813, 0.9946640133857727, -0.3334290087223053, 0.007625999860465527, 0.9427440166473389, -0.608610987663269, 0.04885999858379364, 0.7919629812240601, -0.8253309726715088, 0.14631199836730957, 0.5453640222549438, -0.9105669856071472, 0.3146660029888153, 0.2680560052394867, 0.16523399949073792, 0.04589800164103508, 0.985185980796814, -0.32260099053382874, -0.010471000336110592, 0.9464769959449768, -0.5639140009880066, 0.015166000463068485, 0.8256940245628357, -0.758965015411377, 0.05617399886250496, 0.6487039923667908, -0.8382350206375122, 0.14245299994945526, 0.5263739824295044, 0.9727830290794373, -0.019794000312685966, 0.2308720052242279, 0.9828159809112549, -0.036465998739004135, 0.18095199763774872, 0.9050639867782593, -0.02252200059592724, 0.4246790111064911, 0.8354039788246155, -0.03220000118017197, 0.5486930012702942, 0.6465700268745422, -0.045921001583337784, 0.7614709734916687, 0.4826749861240387, -0.04895399883389473, 0.8744300007820129, 0.4453999996185303, 0.17256900668144226, 0.8785430192947388, 0.47231200337409973, 0.13768500089645386, 0.8706120252609253, 0.4824250042438507, 0.3898639976978302, 0.7843930125236511, 0.641398012638092, 0.4275979995727539, 0.6370000243186951, 0.9863939881324768, 0.0994419977068901, 0.13091400265693665, 0.9154840111732483, 0.2120320051908493, 0.34195101261138916, 0.7246469855308533, 0.245046004652977, 0.6440799832344055, 0.35685500502586365, 0.17885500192642212, 0.9168779850006104, 0.14101800322532654, 0.24263200163841248, 0.9598140120506287, 0.9366779923439026, 0.16484400629997253, 0.3089669942855835, 0.7982620000839233, 0.2441370040178299, 0.5506129860877991, 0.4769439995288849, 0.2904820144176483, 0.8295450210571289, 0.4125959873199463, -0.017246000468730927, 0.9107509851455688, 0.341374009847641, -0.37689098715782166, 0.8610560297966003, 0.9026100039482117, 0.14119599759578705, 0.40664398670196533, 0.7326020002365112, 0.1491979956626892, 0.6641039848327637, 0.38115599751472473, 0.15792299807071686, 0.9109230041503906, 0.5317370295524597, -0.02143399976193905, 0.846638023853302, 0.7915729880332947, -0.3539769947528839, 0.49810999631881714, 0.9087340235710144, -0.07959599792957306, 0.40971601009368896, 0.9386569857597351, -0.17680299282073975, 0.29607900977134705, 0.7781569957733154, -0.19467000663280487, 0.5971400141716003, 0.738847017288208, -0.07674700021743774, 0.6694890260696411, 0.43915998935699463, -0.22276799380779266, 0.870352029800415, 0.3863860070705414, -0.0790880024433136, 0.918940007686615, 0.3576120138168335, 0.07325199991464615, 0.9309930205345154, 0.5227140188217163, 0.16167999804019928, 0.8370360136032104, 0.4246380031108856, 0.3233239948749542, 0.845661997795105, 0.733618974685669, 0.48907899856567383, 0.4718089997768402, 0.9886019825935364, -0.10717800259590149, 0.10573200136423111, 0.9131960272789001, -0.22303399443626404, 0.3410690128803253, 0.7163559794425964, -0.25636500120162964, 0.6489310264587402, 0.35149699449539185, -0.15968100726604462, 0.922469973564148, 0.07999800145626068, -0.2107039988040924, 0.9742709994316101, 0.9883249998092651, 0.04732999950647354, 0.14482200145721436, 0.9210500121116638, 0.07413999736309052, 0.3823229968547821, 0.6804890036582947, 0.11895299702882767, 0.7230389714241028, 0.4935390055179596, -0.10269299894571304, 0.8636389970779419, 0.3912479877471924, -0.4752289950847626, 0.7880880236625671, 0.9700270295143127, 0.07970499992370605, 0.2295520007610321, 0.831250011920929, 0.10592100024223328, 0.5457149744033813, 0.4774230122566223, 0.13252699375152588, 0.8686220049858093, 0.47922399640083313, -0.0024129999801516533, 0.877689003944397, 0.6902980208396912, -0.29711300134658813, 0.6597059965133667, 0.6312130093574524, 0.4550989866256714, 0.628055989742279, 0.26494699716567993, 0.5426689982414246, 0.797065019607544, 0.4216960072517395, 0.6728450059890747, 0.6078259944915771, 0.7324270009994507, 0.5829970240592957, 0.35166099667549133, 0.5086709856987, 0.8606399893760681, -0.023507000878453255, 0.7640720009803772, 0.6449369788169861, -0.015798000618815422, 0.19029100239276886, 0.2773289978504181, -0.9417420029640198, 0.02588699944317341, 0.0005750000127591193, -0.9996650218963623, -0.33045700192451477, -0.4387669861316681, -0.8356329798698425, -0.6271269917488098, -0.4645389914512634, -0.6252319812774658, -0.02311599999666214, 0.2469020038843155, 0.9687650203704834, -0.012950999662280083, 0.45514100790023804, 0.8903250098228455, -0.001180000021122396, 0.9888520240783691, 0.14889399707317352, 0.019520999863743782, 0.6841210126876831, -0.7291070222854614, 0.012253000400960445, 0.007784999907016754, -0.9998949766159058, 0.3314639925956726, -0.3832260072231293, 0.8621309995651245, 0.08274699747562408, -0.16047699749469757, 0.9835649728775024, -0.381630003452301, 0.638043999671936, 0.6687729954719543, -0.44988399744033813, 0.787883996963501, -0.42052799463272095, -0.2780170142650604, 0.5983560085296631, -0.7514500021934509, 0.7378140091896057, -0.4918749928474426, 0.4622659981250763, 0.6153389811515808, -0.45540300011634827, 0.6434019804000854, -0.43678900599479675, 0.33411499857902527, 0.8352140188217163, -0.7429530024528503, 0.6388909816741943, -0.1995989978313446, -0.7432950139045715, 0.5977380275726318, -0.3003700077533722, 0.7197920083999634, 0.5168960094451904, 0.4633769989013672, 0.22183099389076233, 0.4485720098018646, 0.8657789826393127, 0.08203200250864029, 0.15848000347614288, 0.9839479923248291, 0.5953459739685059, 0.4811680018901825, 0.6434599757194519, -0.37556400895118713, -0.6215270161628723, 0.6875, -0.42666301131248474, -0.33534398674964905, 0.8399419784545898, -0.44476398825645447, -0.7887529730796814, -0.42432600259780884, -0.7557309865951538, -0.6222699880599976, -0.20408600568771362, -0.4292669892311096, -0.5361850261688232, -0.7267979979515076, -0.7655900120735168, -0.5671039819717407, -0.30375200510025024, 0.007348000071942806, -0.21113499999046326, 0.9774289727210999, -0.01990099996328354, -0.4373210072517395, 0.8990849852561951, -0.008775000460445881, -0.9864199757575989, 0.16400499641895294, 0.024855999276041985, -0.6829339861869812, -0.7300570011138916, 0.014514000155031681, 0.03655200079083443, -0.9992259740829468, 0.40192899107933044, -0.4676550030708313, 0.7872430086135864, 0.41696101427078247, -0.6813820004463196, 0.6015490293502808, 0.5033609867095947, -0.8637740015983582, -0.022839000448584557, 0.2058819979429245, -0.2945750057697296, -0.9331870079040527, -0.2351589947938919, 0.5535579919815063, -0.7989199757575989, 0.6533820033073425, -0.435588002204895, 0.619156002998352, 0.7555500268936157, -0.554410994052887, 0.3489600121974945, 0.7765160202980042, -0.6298360228538513, -0.01814199984073639, 0.025975000113248825, 0.008264999836683273, -0.9996280074119568, -0.6086519956588745, 0.49536699056625366, -0.6198019981384277, -0.9813200235366821, 0.19238099455833435, 0, -0.831650972366333, 0.5552989840507507, 0, -0.4425640106201172, 0.38308998942375183, 0.8107889890670776, -0.5623509883880615, 0.11026400327682495, 0.8195139765739441, 0.367917001247406, -0.15178599953651428, 0.917385995388031, 0.3960669934749603, -0.07774800062179565, 0.9149240255355835, 0.3283520042896271, -0.13562799990177155, 0.9347670078277588, 0.3530749976634979, -0.06952299922704697, 0.9330080151557922, -0.5935590267181396, 0.20035800337791443, 0.7794510126113892, -0.6201800107955933, 0.12245599925518036, 0.7748429775238037, -0.5552989840507507, 0.831650972366333, 0, -0.2616960108280182, 0.523730993270874, 0.8106920123100281, 0.2819640040397644, -0.2819199860095978, 0.9170699715614319, 0.25169798731803894, -0.25161200761795044, 0.9345269799232483, -0.4710330069065094, 0.41249701380729675, 0.7797269821166992, -0.19238099455833435, 0.9813200235366821, 0, -0.04031100124120712, 0.5840420126914978, 0.8107219934463501, 0.1517850011587143, -0.3678950071334839, 0.9173960089683533, 0.13561999797821045, -0.3282899856567383, 0.9347900152206421, -0.27737799286842346, 0.561601996421814, 0.779528021812439, -1, 0, 0, 0.29074999690055847, 0.5413560271263123, 0.7889220118522644, 0.07767199724912643, -0.39607399702072144, 0.9149270057678223, 0.0693729966878891, -0.3530940115451813, 0.9330130219459534, -0.12221000343561172, 0.6202139854431152, 0.7748550176620483, 0.19238099455833435, 0.9813200235366821, 0, 0.5552989840507507, 0.831650972366333, 0, 0.38308998942375183, 0.4425640106201172, 0.8107889890670776, 0.11026400327682495, 0.5623509883880615, 0.8195139765739441, -0.15178599953651428, -0.367917001247406, 0.917385995388031, -0.07774800062179565, -0.3960669934749603, 0.9149240255355835, -0.13562799990177155, -0.3283520042896271, 0.9347670078277588, -0.06952299922704697, -0.3530749976634979, 0.9330080151557922, 0.20035800337791443, 0.5935590267181396, 0.7794510126113892, 0.12245599925518036, 0.6201800107955933, 0.7748429775238037, 0.831650972366333, 0.5552989840507507, 0, 0.523730993270874, 0.2616960108280182, 0.8106920123100281, -0.2819199860095978, -0.2819640040397644, 0.9170699715614319, -0.25161200761795044, -0.25169798731803894, 0.9345269799232483, 0.41249701380729675, 0.4710330069065094, 0.7797269821166992, 0.9813200235366821, 0.19238099455833435, 0, 0.5840420126914978, 0.04031100124120712, 0.8107219934463501, -0.3678950071334839, -0.1517850011587143, 0.9173960089683533, -0.3282899856567383, -0.13561999797821045, 0.9347900152206421, 0.561601996421814, 0.27737799286842346, 0.779528021812439, -1, 0, 0, 0.5413560271263123, -0.29074999690055847, 0.7889220118522644, -0.39607399702072144, -0.07767199724912643, 0.9149270057678223, -0.3530940115451813, -0.0693729966878891, 0.9330130219459534, 0.6202139854431152, 0.12221000343561172, 0.7748550176620483, 0.9813200235366821, -0.19238099455833435, 0, 0.831650972366333, -0.5552989840507507, 0, 0.4425640106201172, -0.38308998942375183, 0.8107889890670776, 0.5623509883880615, -0.11026400327682495, 0.8195139765739441, -0.367917001247406, 0.15178599953651428, 0.917385995388031, -0.3960669934749603, 0.07774800062179565, 0.9149240255355835, -0.3283520042896271, 0.13562799990177155, 0.9347670078277588, -0.3530749976634979, 0.06952299922704697, 0.9330080151557922, 0.5935590267181396, -0.20035800337791443, 0.7794510126113892, 0.6201800107955933, -0.12245599925518036, 0.7748429775238037, 0.5552989840507507, -0.831650972366333, 0, 0.2616960108280182, -0.523730993270874, 0.8106920123100281, -0.2819640040397644, 0.2819199860095978, 0.9170699715614319, -0.25169798731803894, 0.25161200761795044, 0.9345269799232483, 0.4710330069065094, -0.41249701380729675, 0.7797269821166992, 0.19238099455833435, -0.9813200235366821, 0, 0.04031100124120712, -0.5840420126914978, 0.8107219934463501, -0.1517850011587143, 0.3678950071334839, 0.9173960089683533, -0.13561999797821045, 0.3282899856567383, 0.9347900152206421, 0.27737799286842346, -0.561601996421814, 0.779528021812439, -1, 0, 0, -0.29074999690055847, -0.5413560271263123, 0.7889220118522644, -0.07767199724912643, 0.39607399702072144, 0.9149270057678223, -0.0693729966878891, 0.3530940115451813, 0.9330130219459534, 0.12221000343561172, -0.6202139854431152, 0.7748550176620483, -0.19238099455833435, -0.9813200235366821, 0, -0.5552989840507507, -0.831650972366333, 0, -0.38308998942375183, -0.4425640106201172, 0.8107889890670776, -0.11026400327682495, -0.5623509883880615, 0.8195139765739441, 0.15178599953651428, 0.367917001247406, 0.917385995388031, 0.07774800062179565, 0.3960669934749603, 0.9149240255355835, 0.13562799990177155, 0.3283520042896271, 0.9347670078277588, 0.06952299922704697, 0.3530749976634979, 0.9330080151557922, -0.20035800337791443, -0.5935590267181396, 0.7794510126113892, -0.12245599925518036, -0.6201800107955933, 0.7748429775238037, -0.831650972366333, -0.5552989840507507, 0, -0.523730993270874, -0.2616960108280182, 0.8106920123100281, 0.2819199860095978, 0.2819640040397644, 0.9170699715614319, 0.25161200761795044, 0.25169798731803894, 0.9345269799232483, -0.41249701380729675, -0.4710330069065094, 0.7797269821166992, -0.9813200235366821, -0.19238099455833435, 0, -0.5840420126914978, -0.04031100124120712, 0.8107219934463501, 0.3678950071334839, 0.1517850011587143, 0.9173960089683533, 0.3282899856567383, 0.13561999797821045, 0.9347900152206421, -0.561601996421814, -0.27737799286842346, 0.779528021812439, -1, 0, 0, -0.5413560271263123, 0.29074999690055847, 0.7889220118522644, 0.39607399702072144, 0.07767199724912643, 0.9149270057678223, 0.3530940115451813, 0.0693729966878891, 0.9330130219459534, -0.6202139854431152, -0.12221000343561172, 0.7748550176620483, -0.6201800107955933, 0.12245599925518036, 0.7748429775238037, -0.5616469979286194, 0.27755099534988403, 0.7794349789619446, -0.8979210257530212, 0.37159600853919983, 0.23590999841690063, -0.9542099833488464, 0.18841099739074707, 0.23234599828720093, -0.9101200103759766, 0.3766449987888336, 0.17268399894237518, -0.966795027256012, 0.19089600443840027, 0.16990099847316742, -0.8549879789352417, 0.35383298993110657, 0.3792079985141754, -0.9100499749183655, 0.17969100177288055, 0.3735229969024658, -0.8064150214195251, 0.2724289894104004, 0.5248600244522095, -0.8383409976959229, 0.16553199291229248, 0.5194069743156433, -0.4125959873199463, 0.47094500064849854, 0.7797279953956604, -0.687192976474762, 0.687192976474762, 0.23565199971199036, -0.6965190172195435, 0.6965190172195435, 0.17240400612354279, -0.6544880270957947, 0.6544870138168335, 0.37853899598121643, -0.6404970288276672, 0.5611429810523987, 0.5242909789085388, -0.20047900080680847, 0.5933949947357178, 0.7795450091362, -0.3715969920158386, 0.8979210257530212, 0.23590999841690063, -0.3766449987888336, 0.9101200103759766, 0.17268399894237518, -0.35383298993110657, 0.8549879789352417, 0.3792079985141754, -0.3770729899406433, 0.7630789875984192, 0.5249059796333313, -0.12245900183916092, 0.6201940178871155, 0.7748309969902039, -0.18841099739074707, 0.9542099833488464, 0.23234599828720093, -0.19089600443840027, 0.966795027256012, 0.16990099847316742, -0.17969200015068054, 0.9100499749183655, 0.3735229969024658, -0.16553199291229248, 0.8383409976959229, 0.5194069743156433, 0.12245599925518036, 0.6201800107955933, 0.7748429775238037, 0.27755099534988403, 0.5616469979286194, 0.7794349789619446, 0.37159600853919983, 0.8979210257530212, 0.23590999841690063, 0.18841099739074707, 0.9542099833488464, 0.23234599828720093, 0.3766449987888336, 0.9101200103759766, 0.17268399894237518, 0.19089600443840027, 0.966795027256012, 0.16990099847316742, 0.35383298993110657, 0.8549879789352417, 0.3792079985141754, 0.17969100177288055, 0.9100499749183655, 0.3735229969024658, 0.2724289894104004, 0.8064150214195251, 0.5248600244522095, 0.16553199291229248, 0.8383409976959229, 0.5194069743156433, 0.47094500064849854, 0.4125959873199463, 0.7797279953956604, 0.687192976474762, 0.687192976474762, 0.23565199971199036, 0.6965190172195435, 0.6965190172195435, 0.17240400612354279, 0.6544870138168335, 0.6544880270957947, 0.37853899598121643, 0.5611429810523987, 0.6404970288276672, 0.5242909789085388, 0.5933949947357178, 0.20047900080680847, 0.7795450091362, 0.8979210257530212, 0.3715969920158386, 0.23590999841690063, 0.9101200103759766, 0.3766449987888336, 0.17268399894237518, 0.8549879789352417, 0.35383298993110657, 0.3792079985141754, 0.7630789875984192, 0.3770729899406433, 0.5249059796333313, 0.6201940178871155, 0.12245900183916092, 0.7748309969902039, 0.9542099833488464, 0.18841099739074707, 0.23234599828720093, 0.966795027256012, 0.19089600443840027, 0.16990099847316742, 0.9100499749183655, 0.17969200015068054, 0.3735229969024658, 0.8383409976959229, 0.16553199291229248, 0.5194069743156433, 0.6201800107955933, -0.12245599925518036, 0.7748429775238037, 0.5616469979286194, -0.27755099534988403, 0.7794349789619446, 0.8979210257530212, -0.37159600853919983, 0.23590999841690063, 0.9542099833488464, -0.18841099739074707, 0.23234599828720093, 0.9101200103759766, -0.3766449987888336, 0.17268399894237518, 0.966795027256012, -0.19089600443840027, 0.16990099847316742, 0.8549879789352417, -0.35383298993110657, 0.3792079985141754, 0.9100499749183655, -0.17969100177288055, 0.3735229969024658, 0.8064150214195251, -0.2724289894104004, 0.5248600244522095, 0.8383409976959229, -0.16553199291229248, 0.5194069743156433, 0.4125959873199463, -0.47094500064849854, 0.7797279953956604, 0.687192976474762, -0.687192976474762, 0.23565199971199036, 0.6965190172195435, -0.6965190172195435, 0.17240400612354279, 0.6544880270957947, -0.6544870138168335, 0.37853899598121643, 0.6404970288276672, -0.5611429810523987, 0.5242909789085388, 0.20047900080680847, -0.5933949947357178, 0.7795450091362, 0.3715969920158386, -0.8979210257530212, 0.23590999841690063, 0.3766449987888336, -0.9101200103759766, 0.17268399894237518, 0.35383298993110657, -0.8549879789352417, 0.3792079985141754, 0.3770729899406433, -0.7630789875984192, 0.5249059796333313, 0.12245900183916092, -0.6201940178871155, 0.7748309969902039, 0.18841099739074707, -0.9542099833488464, 0.23234599828720093, 0.19089600443840027, -0.966795027256012, 0.16990099847316742, 0.17969200015068054, -0.9100499749183655, 0.3735229969024658, 0.16553199291229248, -0.8383409976959229, 0.5194069743156433, -0.12245599925518036, -0.6201800107955933, 0.7748429775238037, -0.27755099534988403, -0.5616469979286194, 0.7794349789619446, -0.37159600853919983, -0.8979210257530212, 0.23590999841690063, -0.18841099739074707, -0.9542099833488464, 0.23234599828720093, -0.3766449987888336, -0.9101200103759766, 0.17268399894237518, -0.19089600443840027, -0.966795027256012, 0.16990099847316742, -0.35383298993110657, -0.8549879789352417, 0.3792079985141754, -0.17969100177288055, -0.9100499749183655, 0.3735229969024658, -0.2724289894104004, -0.8064150214195251, 0.5248600244522095, -0.16553199291229248, -0.8383409976959229, 0.5194069743156433, -0.47094500064849854, -0.4125959873199463, 0.7797279953956604, -0.687192976474762, -0.687192976474762, 0.23565199971199036, -0.6965190172195435, -0.6965190172195435, 0.17240400612354279, -0.6544870138168335, -0.6544880270957947, 0.37853899598121643, -0.5611429810523987, -0.6404970288276672, 0.5242909789085388, -0.5933949947357178, -0.20047900080680847, 0.7795450091362, -0.8979210257530212, -0.3715969920158386, 0.23590999841690063, -0.9101200103759766, -0.3766449987888336, 0.17268399894237518, -0.8549879789352417, -0.35383298993110657, 0.3792079985141754, -0.7630789875984192, -0.3770729899406433, 0.5249059796333313, -0.6201940178871155, -0.12245900183916092, 0.7748309969902039, -0.9542099833488464, -0.18841099739074707, 0.23234599828720093, -0.966795027256012, -0.19089600443840027, 0.16990099847316742, -0.9100499749183655, -0.17969200015068054, 0.3735229969024658, -0.8383409976959229, -0.16553199291229248, 0.5194069743156433
  };
  core->addAttributeData(OGLConstants::BINORMAL.location, binormals,
      sizeof(binormals), 3, GL_STATIC_DRAW);

  // define texture coordinates (3D)
  GLfloat texCoords[] = {
      2, 2, 0, 1.75, 2, 0, 1.75, 1.975000023841858, 0, 2, 1.975000023841858, 0, 1.75, 1.9500000476837158, 0, 2, 1.9500000476837158, 0, 1.75, 1.9249999523162842, 0, 2, 1.9249999523162842, 0, 1.75, 1.899999976158142, 0, 2, 1.899999976158142, 0, 1.5, 2, 0, 1.5, 1.975000023841858, 0, 1.5, 1.9500000476837158, 0, 1.5, 1.9249999523162842, 0, 1.5, 1.899999976158142, 0, 1.25, 2, 0, 1.25, 1.975000023841858, 0, 1.25, 1.9500000476837158, 0, 1.25, 1.9249999523162842, 0, 1.25, 1.899999976158142, 0, 1, 2, 0, 1, 1.975000023841858, 0, 1, 1.9500000476837158, 0, 1, 1.9249999523162842, 0, 1, 1.899999976158142, 0, 1, 2, 0, 0.75, 2, 0, 0.75, 1.975000023841858, 0, 1, 1.975000023841858, 0, 0.75, 1.9500000476837158, 0, 1, 1.9500000476837158, 0, 0.75, 1.9249999523162842, 0, 1, 1.9249999523162842, 0, 0.75, 1.899999976158142, 0, 1, 1.899999976158142, 0, 0.5, 2, 0, 0.5, 1.975000023841858, 0, 0.5, 1.9500000476837158, 0, 0.5, 1.9249999523162842, 0, 0.5, 1.899999976158142, 0, 0.25, 2, 0, 0.25, 1.975000023841858, 0, 0.25, 1.9500000476837158, 0, 0.25, 1.9249999523162842, 0, 0.25, 1.899999976158142, 0, 0, 2, 0, 0, 1.975000023841858, 0, 0, 1.9500000476837158, 0, 0, 1.9249999523162842, 0, 0, 1.899999976158142, 0, 2, 2, 0, 1.75, 2, 0, 1.75, 1.975000023841858, 0, 2, 1.975000023841858, 0, 1.75, 1.9500000476837158, 0, 2, 1.9500000476837158, 0, 1.75, 1.9249999523162842, 0, 2, 1.9249999523162842, 0, 1.75, 1.899999976158142, 0, 2, 1.899999976158142, 0, 1.5, 2, 0, 1.5, 1.975000023841858, 0, 1.5, 1.9500000476837158, 0, 1.5, 1.9249999523162842, 0, 1.5, 1.899999976158142, 0, 1.25, 2, 0, 1.25, 1.975000023841858, 0, 1.25, 1.9500000476837158, 0, 1.25, 1.9249999523162842, 0, 1.25, 1.899999976158142, 0, 1, 2, 0, 1, 1.975000023841858, 0, 1, 1.9500000476837158, 0, 1, 1.9249999523162842, 0, 1, 1.899999976158142, 0, 1, 2, 0, 0.75, 2, 0, 0.75, 1.975000023841858, 0, 1, 1.975000023841858, 0, 0.75, 1.9500000476837158, 0, 1, 1.9500000476837158, 0, 0.75, 1.9249999523162842, 0, 1, 1.9249999523162842, 0, 0.75, 1.899999976158142, 0, 1, 1.899999976158142, 0, 0.5, 2, 0, 0.5, 1.975000023841858, 0, 0.5, 1.9500000476837158, 0, 0.5, 1.9249999523162842, 0, 0.5, 1.899999976158142, 0, 0.25, 2, 0, 0.25, 1.975000023841858, 0, 0.25, 1.9500000476837158, 0, 0.25, 1.9249999523162842, 0, 0.25, 1.899999976158142, 0, 0, 2, 0, 0, 1.975000023841858, 0, 0, 1.9500000476837158, 0, 0, 1.9249999523162842, 0, 0, 1.899999976158142, 0, 2, 1.899999976158142, 0, 1.75, 1.899999976158142, 0, 1.75, 1.6749999523162842, 0, 2, 1.6749999523162842, 0, 1.75, 1.4500000476837158, 0, 2, 1.4500000476837158, 0, 1.75, 1.225000023841858, 0, 2, 1.225000023841858, 0, 1.75, 1, 0, 2, 1, 0, 1.5, 1.899999976158142, 0, 1.5, 1.6749999523162842, 0, 1.5, 1.4500000476837158, 0, 1.5, 1.225000023841858, 0, 1.5, 1, 0, 1.25, 1.899999976158142, 0, 1.25, 1.6749999523162842, 0, 1.25, 1.4500000476837158, 0, 1.25, 1.225000023841858, 0, 1.25, 1, 0, 1, 1.899999976158142, 0, 1, 1.6749999523162842, 0, 1, 1.4500000476837158, 0, 1, 1.225000023841858, 0, 1, 1, 0, 1, 1.899999976158142, 0, 0.75, 1.899999976158142, 0, 0.75, 1.6749999523162842, 0, 1, 1.6749999523162842, 0, 0.75, 1.4500000476837158, 0, 1, 1.4500000476837158, 0, 0.75, 1.225000023841858, 0, 1, 1.225000023841858, 0, 0.75, 1, 0, 1, 1, 0, 0.5, 1.899999976158142, 0, 0.5, 1.6749999523162842, 0, 0.5, 1.4500000476837158, 0, 0.5, 1.225000023841858, 0, 0.5, 1, 0, 0.25, 1.899999976158142, 0, 0.25, 1.6749999523162842, 0, 0.25, 1.4500000476837158, 0, 0.25, 1.225000023841858, 0, 0.25, 1, 0, 0, 1.899999976158142, 0, 0, 1.6749999523162842, 0, 0, 1.4500000476837158, 0, 0, 1.225000023841858, 0, 0, 1, 0, 2, 1.899999976158142, 0, 1.75, 1.899999976158142, 0, 1.75, 1.6749999523162842, 0, 2, 1.6749999523162842, 0, 1.75, 1.4500000476837158, 0, 2, 1.4500000476837158, 0, 1.75, 1.225000023841858, 0, 2, 1.225000023841858, 0, 1.75, 1, 0, 2, 1, 0, 1.5, 1.899999976158142, 0, 1.5, 1.6749999523162842, 0, 1.5, 1.4500000476837158, 0, 1.5, 1.225000023841858, 0, 1.5, 1, 0, 1.25, 1.899999976158142, 0, 1.25, 1.6749999523162842, 0, 1.25, 1.4500000476837158, 0, 1.25, 1.225000023841858, 0, 1.25, 1, 0, 1, 1.899999976158142, 0, 1, 1.6749999523162842, 0, 1, 1.4500000476837158, 0, 1, 1.225000023841858, 0, 1, 1, 0, 1, 1.899999976158142, 0, 0.75, 1.899999976158142, 0, 0.75, 1.6749999523162842, 0, 1, 1.6749999523162842, 0, 0.75, 1.4500000476837158, 0, 1, 1.4500000476837158, 0, 0.75, 1.225000023841858, 0, 1, 1.225000023841858, 0, 0.75, 1, 0, 1, 1, 0, 0.5, 1.899999976158142, 0, 0.5, 1.6749999523162842, 0, 0.5, 1.4500000476837158, 0, 0.5, 1.225000023841858, 0, 0.5, 1, 0, 0.25, 1.899999976158142, 0, 0.25, 1.6749999523162842, 0, 0.25, 1.4500000476837158, 0, 0.25, 1.225000023841858, 0, 0.25, 1, 0, 0, 1.899999976158142, 0, 0, 1.6749999523162842, 0, 0, 1.4500000476837158, 0, 0, 1.225000023841858, 0, 0, 1, 0, 2, 1, 0, 1.75, 1, 0, 1.75, 0.8500000238418579, 0, 2, 0.8500000238418579, 0, 1.75, 0.699999988079071, 0, 2, 0.699999988079071, 0, 1.75, 0.550000011920929, 0, 2, 0.550000011920929, 0, 1.75, 0.4000000059604645, 0, 2, 0.4000000059604645, 0, 1.5, 1, 0, 1.5, 0.8500000238418579, 0, 1.5, 0.699999988079071, 0, 1.5, 0.550000011920929, 0, 1.5, 0.4000000059604645, 0, 1.25, 1, 0, 1.25, 0.8500000238418579, 0, 1.25, 0.699999988079071, 0, 1.25, 0.550000011920929, 0, 1.25, 0.4000000059604645, 0, 1, 1, 0, 1, 0.8500000238418579, 0, 1, 0.699999988079071, 0, 1, 0.550000011920929, 0, 1, 0.4000000059604645, 0, 1, 1, 0, 0.75, 1, 0, 0.75, 0.8500000238418579, 0, 1, 0.8500000238418579, 0, 0.75, 0.699999988079071, 0, 1, 0.699999988079071, 0, 0.75, 0.550000011920929, 0, 1, 0.550000011920929, 0, 0.75, 0.4000000059604645, 0, 1, 0.4000000059604645, 0, 0.5, 1, 0, 0.5, 0.8500000238418579, 0, 0.5, 0.699999988079071, 0, 0.5, 0.550000011920929, 0, 0.5, 0.4000000059604645, 0, 0.25, 1, 0, 0.25, 0.8500000238418579, 0, 0.25, 0.699999988079071, 0, 0.25, 0.550000011920929, 0, 0.25, 0.4000000059604645, 0, 0, 1, 0, 0, 0.8500000238418579, 0, 0, 0.699999988079071, 0, 0, 0.550000011920929, 0, 0, 0.4000000059604645, 0, 2, 1, 0, 1.75, 1, 0, 1.75, 0.8500000238418579, 0, 2, 0.8500000238418579, 0, 1.75, 0.699999988079071, 0, 2, 0.699999988079071, 0, 1.75, 0.550000011920929, 0, 2, 0.550000011920929, 0, 1.75, 0.4000000059604645, 0, 2, 0.4000000059604645, 0, 1.5, 1, 0, 1.5, 0.8500000238418579, 0, 1.5, 0.699999988079071, 0, 1.5, 0.550000011920929, 0, 1.5, 0.4000000059604645, 0, 1.25, 1, 0, 1.25, 0.8500000238418579, 0, 1.25, 0.699999988079071, 0, 1.25, 0.550000011920929, 0, 1.25, 0.4000000059604645, 0, 1, 1, 0, 1, 0.8500000238418579, 0, 1, 0.699999988079071, 0, 1, 0.550000011920929, 0, 1, 0.4000000059604645, 0, 1, 1, 0, 0.75, 1, 0, 0.75, 0.8500000238418579, 0, 1, 0.8500000238418579, 0, 0.75, 0.699999988079071, 0, 1, 0.699999988079071, 0, 0.75, 0.550000011920929, 0, 1, 0.550000011920929, 0, 0.75, 0.4000000059604645, 0, 1, 0.4000000059604645, 0, 0.5, 1, 0, 0.5, 0.8500000238418579, 0, 0.5, 0.699999988079071, 0, 0.5, 0.550000011920929, 0, 0.5, 0.4000000059604645, 0, 0.25, 1, 0, 0.25, 0.8500000238418579, 0, 0.25, 0.699999988079071, 0, 0.25, 0.550000011920929, 0, 0.25, 0.4000000059604645, 0, 0, 1, 0, 0, 0.8500000238418579, 0, 0, 0.699999988079071, 0, 0, 0.550000011920929, 0, 0, 0.4000000059604645, 0, 2, 0.4000000059604645, 0, 1.75, 0.4000000059604645, 0, 1.75, 0.30000001192092896, 0, 2, 0.30000001192092896, 0, 1.75, 0.20000000298023224, 0, 2, 0.20000000298023224, 0, 1.75, 0.10000000149011612, 0, 2, 0.10000000149011612, 0, 1.75, 0, 0, 2, 0, 0, 1.5, 0.4000000059604645, 0, 1.5, 0.30000001192092896, 0, 1.5, 0.20000000298023224, 0, 1.5, 0.10000000149011612, 0, 1.5, 0, 0, 1.25, 0.4000000059604645, 0, 1.25, 0.30000001192092896, 0, 1.25, 0.20000000298023224, 0, 1.25, 0.10000000149011612, 0, 1.25, 0, 0, 1, 0.4000000059604645, 0, 1, 0.30000001192092896, 0, 1, 0.20000000298023224, 0, 1, 0.10000000149011612, 0, 1, 0, 0, 1, 0.4000000059604645, 0, 0.75, 0.4000000059604645, 0, 0.75, 0.30000001192092896, 0, 1, 0.30000001192092896, 0, 0.75, 0.20000000298023224, 0, 1, 0.20000000298023224, 0, 0.75, 0.10000000149011612, 0, 1, 0.10000000149011612, 0, 0.75, 0, 0, 1, 0, 0, 0.5, 0.4000000059604645, 0, 0.5, 0.30000001192092896, 0, 0.5, 0.20000000298023224, 0, 0.5, 0.10000000149011612, 0, 0.5, 0, 0, 0.25, 0.4000000059604645, 0, 0.25, 0.30000001192092896, 0, 0.25, 0.20000000298023224, 0, 0.25, 0.10000000149011612, 0, 0.25, 0, 0, 0, 0.4000000059604645, 0, 0, 0.30000001192092896, 0, 0, 0.20000000298023224, 0, 0, 0.10000000149011612, 0, 0, 0, 0, 2, 0.4000000059604645, 0, 1.75, 0.4000000059604645, 0, 1.75, 0.30000001192092896, 0, 2, 0.30000001192092896, 0, 1.75, 0.20000000298023224, 0, 2, 0.20000000298023224, 0, 1.75, 0.10000000149011612, 0, 2, 0.10000000149011612, 0, 1.75, 0, 0, 2, 0, 0, 1.5, 0.4000000059604645, 0, 1.5, 0.30000001192092896, 0, 1.5, 0.20000000298023224, 0, 1.5, 0.10000000149011612, 0, 1.5, 0, 0, 1.25, 0.4000000059604645, 0, 1.25, 0.30000001192092896, 0, 1.25, 0.20000000298023224, 0, 1.25, 0.10000000149011612, 0, 1.25, 0, 0, 1, 0.4000000059604645, 0, 1, 0.30000001192092896, 0, 1, 0.20000000298023224, 0, 1, 0.10000000149011612, 0, 1, 0, 0, 1, 0.4000000059604645, 0, 0.75, 0.4000000059604645, 0, 0.75, 0.30000001192092896, 0, 1, 0.30000001192092896, 0, 0.75, 0.20000000298023224, 0, 1, 0.20000000298023224, 0, 0.75, 0.10000000149011612, 0, 1, 0.10000000149011612, 0, 0.75, 0, 0, 1, 0, 0, 0.5, 0.4000000059604645, 0, 0.5, 0.30000001192092896, 0, 0.5, 0.20000000298023224, 0, 0.5, 0.10000000149011612, 0, 0.5, 0, 0, 0.25, 0.4000000059604645, 0, 0.25, 0.30000001192092896, 0, 0.25, 0.20000000298023224, 0, 0.25, 0.10000000149011612, 0, 0.25, 0, 0, 0, 0.4000000059604645, 0, 0, 0.30000001192092896, 0, 0, 0.20000000298023224, 0, 0, 0.10000000149011612, 0, 0, 0, 0, 1, 1, 0, 0.875, 1, 0, 0.875, 0.875, 0, 1, 0.875, 0, 0.875, 0.75, 0, 1, 0.75, 0, 0.875, 0.625, 0, 1, 0.625, 0, 0.875, 0.5, 0, 1, 0.5, 0, 0.75, 1, 0, 0.75, 0.875, 0, 0.75, 0.75, 0, 0.75, 0.625, 0, 0.75, 0.5, 0, 0.625, 1, 0, 0.625, 0.875, 0, 0.625, 0.75, 0, 0.625, 0.625, 0, 0.625, 0.5, 0, 0.5, 1, 0, 0.5, 0.875, 0, 0.5, 0.75, 0, 0.5, 0.625, 0, 0.5, 0.5, 0, 0.5, 1, 0, 0.375, 1, 0, 0.375, 0.875, 0, 0.5, 0.875, 0, 0.375, 0.75, 0, 0.5, 0.75, 0, 0.375, 0.625, 0, 0.5, 0.625, 0, 0.375, 0.5, 0, 0.5, 0.5, 0, 0.25, 1, 0, 0.25, 0.875, 0, 0.25, 0.75, 0, 0.25, 0.625, 0, 0.25, 0.5, 0, 0.125, 1, 0, 0.125, 0.875, 0, 0.125, 0.75, 0, 0.125, 0.625, 0, 0.125, 0.5, 0, 0, 1, 0, 0, 0.875, 0, 0, 0.75, 0, 0, 0.625, 0, 0, 0.5, 0, 1, 0.5, 0, 0.875, 0.5, 0, 0.875, 0.375, 0, 1, 0.375, 0, 0.875, 0.25, 0, 1, 0.25, 0, 0.875, 0.125, 0, 1, 0.125, 0, 0.875, 0, 0, 1, 0, 0, 0.75, 0.5, 0, 0.75, 0.375, 0, 0.75, 0.25, 0, 0.75, 0.125, 0, 0.75, 0, 0, 0.625, 0.5, 0, 0.625, 0.375, 0, 0.625, 0.25, 0, 0.625, 0.125, 0, 0.625, 0, 0, 0.5, 0.5, 0, 0.5, 0.375, 0, 0.5, 0.25, 0, 0.5, 0.125, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0.375, 0.5, 0, 0.375, 0.375, 0, 0.5, 0.375, 0, 0.375, 0.25, 0, 0.5, 0.25, 0, 0.375, 0.125, 0, 0.5, 0.125, 0, 0.375, 0, 0, 0.5, 0, 0, 0.25, 0.5, 0, 0.25, 0.375, 0, 0.25, 0.25, 0, 0.25, 0.125, 0, 0.25, 0, 0, 0.125, 0.5, 0, 0.125, 0.375, 0, 0.125, 0.25, 0, 0.125, 0.125, 0, 0.125, 0, 0, 0, 0.5, 0, 0, 0.375, 0, 0, 0.25, 0, 0, 0.125, 0, 0, 0, 0, 0.5, 0, 0, 0.625, 0, 0, 0.625, 0.22499999403953552, 0, 0.5, 0.22499999403953552, 0, 0.625, 0.44999998807907104, 0, 0.5, 0.44999998807907104, 0, 0.625, 0.675000011920929, 0, 0.5, 0.675000011920929, 0, 0.625, 0.8999999761581421, 0, 0.5, 0.8999999761581421, 0, 0.75, 0, 0, 0.75, 0.22499999403953552, 0, 0.75, 0.44999998807907104, 0, 0.75, 0.675000011920929, 0, 0.75, 0.8999999761581421, 0, 0.875, 0, 0, 0.875, 0.22499999403953552, 0, 0.875, 0.44999998807907104, 0, 0.875, 0.675000011920929, 0, 0.875, 0.8999999761581421, 0, 1, 0, 0, 1, 0.22499999403953552, 0, 1, 0.44999998807907104, 0, 1, 0.675000011920929, 0, 1, 0.8999999761581421, 0, 0, 0, 0, 0.125, 0, 0, 0.125, 0.22499999403953552, 0, 0, 0.22499999403953552, 0, 0.125, 0.44999998807907104, 0, 0, 0.44999998807907104, 0, 0.125, 0.675000011920929, 0, 0, 0.675000011920929, 0, 0.125, 0.8999999761581421, 0, 0, 0.8999999761581421, 0, 0.25, 0, 0, 0.25, 0.22499999403953552, 0, 0.25, 0.44999998807907104, 0, 0.25, 0.675000011920929, 0, 0.25, 0.8999999761581421, 0, 0.375, 0, 0, 0.375, 0.22499999403953552, 0, 0.375, 0.44999998807907104, 0, 0.375, 0.675000011920929, 0, 0.375, 0.8999999761581421, 0, 0.5, 0, 0, 0.5, 0.22499999403953552, 0, 0.5, 0.44999998807907104, 0, 0.5, 0.675000011920929, 0, 0.5, 0.8999999761581421, 0, 0.5, 0.8999999761581421, 0, 0.625, 0.8999999761581421, 0, 0.625, 0.925000011920929, 0, 0.5, 0.925000011920929, 0, 0.625, 0.949999988079071, 0, 0.5, 0.949999988079071, 0, 0.625, 0.9750000238418579, 0, 0.5, 0.9750000238418579, 0, 0.625, 1, 0, 0.5, 1, 0, 0.75, 0.8999999761581421, 0, 0.75, 0.925000011920929, 0, 0.75, 0.949999988079071, 0, 0.75, 0.9750000238418579, 0, 0.75, 1, 0, 0.875, 0.8999999761581421, 0, 0.875, 0.925000011920929, 0, 0.875, 0.949999988079071, 0, 0.875, 0.9750000238418579, 0, 0.875, 1, 0, 1, 0.8999999761581421, 0, 1, 0.925000011920929, 0, 1, 0.949999988079071, 0, 1, 0.9750000238418579, 0, 1, 1, 0, 0, 0.8999999761581421, 0, 0.125, 0.8999999761581421, 0, 0.125, 0.925000011920929, 0, 0, 0.925000011920929, 0, 0.125, 0.949999988079071, 0, 0, 0.949999988079071, 0, 0.125, 0.9750000238418579, 0, 0, 0.9750000238418579, 0, 0.125, 1, 0, 0, 1, 0, 0.25, 0.8999999761581421, 0, 0.25, 0.925000011920929, 0, 0.25, 0.949999988079071, 0, 0.25, 0.9750000238418579, 0, 0.25, 1, 0, 0.375, 0.8999999761581421, 0, 0.375, 0.925000011920929, 0, 0.375, 0.949999988079071, 0, 0.375, 0.9750000238418579, 0, 0.375, 1, 0, 0.5, 0.8999999761581421, 0, 0.5, 0.925000011920929, 0, 0.5, 0.949999988079071, 0, 0.5, 0.9750000238418579, 0, 0.5, 1, 0, 1, 1, 0, 0.875, 1, 0, 0.875, 0.75, 0, 1, 0.75, 0, 0.875, 0.5, 0, 1, 0.5, 0, 0.875, 0.25, 0, 1, 0.25, 0, 0.875, 0, 0, 1, 0, 0, 0.75, 1, 0, 0.75, 0.75, 0, 0.75, 0.5, 0, 0.75, 0.25, 0, 0.75, 0, 0, 0.625, 1, 0, 0.625, 0.75, 0, 0.625, 0.5, 0, 0.625, 0.25, 0, 0.625, 0, 0, 0.5, 1, 0, 0.5, 0.75, 0, 0.5, 0.5, 0, 0.5, 0.25, 0, 0.5, 0, 0, 0.5, 1, 0, 0.375, 1, 0, 0.375, 0.75, 0, 0.5, 0.75, 0, 0.375, 0.5, 0, 0.5, 0.5, 0, 0.375, 0.25, 0, 0.5, 0.25, 0, 0.375, 0, 0, 0.5, 0, 0, 0.25, 1, 0, 0.25, 0.75, 0, 0.25, 0.5, 0, 0.25, 0.25, 0, 0.25, 0, 0, 0.125, 1, 0, 0.125, 0.75, 0, 0.125, 0.5, 0, 0.125, 0.25, 0, 0.125, 0, 0, 0, 1, 0, 0, 0.75, 0, 0, 0.5, 0, 0, 0.25, 0, 0, 0, 0, 1, 1, 0, 0.875, 1, 0, 0.875, 0.75, 0, 1, 0.75, 0, 0.875, 0.5, 0, 1, 0.5, 0, 0.875, 0.25, 0, 1, 0.25, 0, 0.875, 0, 0, 1, 0, 0, 0.75, 1, 0, 0.75, 0.75, 0, 0.75, 0.5, 0, 0.75, 0.25, 0, 0.75, 0, 0, 0.625, 1, 0, 0.625, 0.75, 0, 0.625, 0.5, 0, 0.625, 0.25, 0, 0.625, 0, 0, 0.5, 1, 0, 0.5, 0.75, 0, 0.5, 0.5, 0, 0.5, 0.25, 0, 0.5, 0, 0, 0.5, 1, 0, 0.375, 1, 0, 0.375, 0.75, 0, 0.5, 0.75, 0, 0.375, 0.5, 0, 0.5, 0.5, 0, 0.375, 0.25, 0, 0.5, 0.25, 0, 0.375, 0, 0, 0.5, 0, 0, 0.25, 1, 0, 0.25, 0.75, 0, 0.25, 0.5, 0, 0.25, 0.25, 0, 0.25, 0, 0, 0.125, 1, 0, 0.125, 0.75, 0, 0.125, 0.5, 0, 0.125, 0.25, 0, 0.125, 0, 0, 0, 1, 0, 0, 0.75, 0, 0, 0.5, 0, 0, 0.25, 0, 0, 0, 0, 1, 1, 0, 0.875, 1, 0, 0.875, 0.75, 0, 1, 0.75, 0, 0.875, 0.5, 0, 1, 0.5, 0, 0.875, 0.25, 0, 1, 0.25, 0, 0.875, 0, 0, 1, 0, 0, 0.75, 1, 0, 0.75, 0.75, 0, 0.75, 0.5, 0, 0.75, 0.25, 0, 0.75, 0, 0, 0.625, 1, 0, 0.625, 0.75, 0, 0.625, 0.5, 0, 0.625, 0.25, 0, 0.625, 0, 0, 0.5, 1, 0, 0.5, 0.75, 0, 0.5, 0.5, 0, 0.5, 0.25, 0, 0.5, 0, 0, 0.5, 1, 0, 0.375, 1, 0, 0.375, 0.75, 0, 0.5, 0.75, 0, 0.375, 0.5, 0, 0.5, 0.5, 0, 0.375, 0.25, 0, 0.5, 0.25, 0, 0.375, 0, 0, 0.5, 0, 0, 0.25, 1, 0, 0.25, 0.75, 0, 0.25, 0.5, 0, 0.25, 0.25, 0, 0.25, 0, 0, 0.125, 1, 0, 0.125, 0.75, 0, 0.125, 0.5, 0, 0.125, 0.25, 0, 0.125, 0, 0, 0, 1, 0, 0, 0.75, 0, 0, 0.5, 0, 0, 0.25, 0, 0, 0, 0, 1, 1, 0, 0.875, 1, 0, 0.875, 0.75, 0, 1, 0.75, 0, 0.875, 0.5, 0, 1, 0.5, 0, 0.875, 0.25, 0, 1, 0.25, 0, 0.875, 0, 0, 1, 0, 0, 0.75, 1, 0, 0.75, 0.75, 0, 0.75, 0.5, 0, 0.75, 0.25, 0, 0.75, 0, 0, 0.625, 1, 0, 0.625, 0.75, 0, 0.625, 0.5, 0, 0.625, 0.25, 0, 0.625, 0, 0, 0.5, 1, 0, 0.5, 0.75, 0, 0.5, 0.5, 0, 0.5, 0.25, 0, 0.5, 0, 0, 0.5, 1, 0, 0.375, 1, 0, 0.375, 0.75, 0, 0.5, 0.75, 0, 0.375, 0.5, 0, 0.5, 0.5, 0, 0.375, 0.25, 0, 0.5, 0.25, 0, 0.375, 0, 0, 0.5, 0, 0, 0.25, 1, 0, 0.25, 0.75, 0, 0.25, 0.5, 0, 0.25, 0.25, 0, 0.25, 0, 0, 0.125, 1, 0, 0.125, 0.75, 0, 0.125, 0.5, 0, 0.125, 0.25, 0, 0.125, 0, 0, 0, 1, 0, 0, 0.75, 0, 0, 0.5, 0, 0, 0.25, 0, 0, 0, 0
  };
  // mirror texture coordinates
  int nTexCoords = sizeof(texCoords) / sizeof(GLfloat);
  for (int i = 1; i < nTexCoords; i += 3) {
    texCoords[i] = 2.f - texCoords[i];
  }
  core->addAttributeData(OGLConstants::TEX_COORD_0.location, texCoords,
      sizeof(texCoords), 3, GL_STATIC_DRAW);

  // define indices (1024 triangles)
  const GLuint indices[] = {
      0, 1, 2, 2, 3, 0, 3, 2, 4, 4, 5, 3, 5, 4, 6, 6, 7, 5, 7, 6, 8, 8, 9, 7, 1, 10, 11, 11, 2, 1, 2, 11, 12, 12, 4, 2, 4, 12, 13, 13, 6, 4, 6, 13, 14, 14, 8, 6, 10, 15, 16, 16, 11, 10, 11, 16, 17, 17, 12, 11, 12, 17, 18, 18, 13, 12, 13, 18, 19, 19, 14, 13, 15, 20, 21, 21, 16, 15, 16, 21, 22, 22, 17, 16, 17, 22, 23, 23, 18, 17, 18, 23, 24, 24, 19, 18, 25, 26, 27, 27, 28, 25, 28, 27, 29, 29, 30, 28, 30, 29, 31, 31, 32, 30, 32, 31, 33, 33, 34, 32, 26, 35, 36, 36, 27, 26, 27, 36, 37, 37, 29, 27, 29, 37, 38, 38, 31, 29, 31, 38, 39, 39, 33, 31, 35, 40, 41, 41, 36, 35, 36, 41, 42, 42, 37, 36, 37, 42, 43, 43, 38, 37, 38, 43, 44, 44, 39, 38, 40, 45, 46, 46, 41, 40, 41, 46, 47, 47, 42, 41, 42, 47, 48, 48, 43, 42, 43, 48, 49, 49, 44, 43, 50, 51, 52, 52, 53, 50, 53, 52, 54, 54, 55, 53, 55, 54, 56, 56, 57, 55, 57, 56, 58, 58, 59, 57, 51, 60, 61, 61, 52, 51, 52, 61, 62, 62, 54, 52, 54, 62, 63, 63, 56, 54, 56, 63, 64, 64, 58, 56, 60, 65, 66, 66, 61, 60, 61, 66, 67, 67, 62, 61, 62, 67, 68, 68, 63, 62, 63, 68, 69, 69, 64, 63, 65, 70, 71, 71, 66, 65, 66, 71, 72, 72, 67, 66, 67, 72, 73, 73, 68, 67, 68, 73, 74, 74, 69, 68, 75, 76, 77, 77, 78, 75, 78, 77, 79, 79, 80, 78, 80, 79, 81, 81, 82, 80, 82, 81, 83, 83, 84, 82, 76, 85, 86, 86, 77, 76, 77, 86, 87, 87, 79, 77, 79, 87, 88, 88, 81, 79, 81, 88, 89, 89, 83, 81, 85, 90, 91, 91, 86, 85, 86, 91, 92, 92, 87, 86, 87, 92, 93, 93, 88, 87, 88, 93, 94, 94, 89, 88, 90, 95, 96, 96, 91, 90, 91, 96, 97, 97, 92, 91, 92, 97, 98, 98, 93, 92, 93, 98, 99, 99, 94, 93, 100, 101, 102, 102, 103, 100, 103, 102, 104, 104, 105, 103, 105, 104, 106, 106, 107, 105, 107, 106, 108, 108, 109, 107, 101, 110, 111, 111, 102, 101, 102, 111, 112, 112, 104, 102, 104, 112, 113, 113, 106, 104, 106, 113, 114, 114, 108, 106, 110, 115, 116, 116, 111, 110, 111, 116, 117, 117, 112, 111, 112, 117, 118, 118, 113, 112, 113, 118, 119, 119, 114, 113, 115, 120, 121, 121, 116, 115, 116, 121, 122, 122, 117, 116, 117, 122, 123, 123, 118, 117, 118, 123, 124, 124, 119, 118, 125, 126, 127, 127, 128, 125, 128, 127, 129, 129, 130, 128, 130, 129, 131, 131, 132, 130, 132, 131, 133, 133, 134, 132, 126, 135, 136, 136, 127, 126, 127, 136, 137, 137, 129, 127, 129, 137, 138, 138, 131, 129, 131, 138, 139, 139, 133, 131, 135, 140, 141, 141, 136, 135, 136, 141, 142, 142, 137, 136, 137, 142, 143, 143, 138, 137, 138, 143, 144, 144, 139, 138, 140, 145, 146, 146, 141, 140, 141, 146, 147, 147, 142, 141, 142, 147, 148, 148, 143, 142, 143, 148, 149, 149, 144, 143, 150, 151, 152, 152, 153, 150, 153, 152, 154, 154, 155, 153, 155, 154, 156, 156, 157, 155, 157, 156, 158, 158, 159, 157, 151, 160, 161, 161, 152, 151, 152, 161, 162, 162, 154, 152, 154, 162, 163, 163, 156, 154, 156, 163, 164, 164, 158, 156, 160, 165, 166, 166, 161, 160, 161, 166, 167, 167, 162, 161, 162, 167, 168, 168, 163, 162, 163, 168, 169, 169, 164, 163, 165, 170, 171, 171, 166, 165, 166, 171, 172, 172, 167, 166, 167, 172, 173, 173, 168, 167, 168, 173, 174, 174, 169, 168, 175, 176, 177, 177, 178, 175, 178, 177, 179, 179, 180, 178, 180, 179, 181, 181, 182, 180, 182, 181, 183, 183, 184, 182, 176, 185, 186, 186, 177, 176, 177, 186, 187, 187, 179, 177, 179, 187, 188, 188, 181, 179, 181, 188, 189, 189, 183, 181, 185, 190, 191, 191, 186, 185, 186, 191, 192, 192, 187, 186, 187, 192, 193, 193, 188, 187, 188, 193, 194, 194, 189, 188, 190, 195, 196, 196, 191, 190, 191, 196, 197, 197, 192, 191, 192, 197, 198, 198, 193, 192, 193, 198, 199, 199, 194, 193, 200, 201, 202, 202, 203, 200, 203, 202, 204, 204, 205, 203, 205, 204, 206, 206, 207, 205, 207, 206, 208, 208, 209, 207, 201, 210, 211, 211, 202, 201, 202, 211, 212, 212, 204, 202, 204, 212, 213, 213, 206, 204, 206, 213, 214, 214, 208, 206, 210, 215, 216, 216, 211, 210, 211, 216, 217, 217, 212, 211, 212, 217, 218, 218, 213, 212, 213, 218, 219, 219, 214, 213, 215, 220, 221, 221, 216, 215, 216, 221, 222, 222, 217, 216, 217, 222, 223, 223, 218, 217, 218, 223, 224, 224, 219, 218, 225, 226, 227, 227, 228, 225, 228, 227, 229, 229, 230, 228, 230, 229, 231, 231, 232, 230, 232, 231, 233, 233, 234, 232, 226, 235, 236, 236, 227, 226, 227, 236, 237, 237, 229, 227, 229, 237, 238, 238, 231, 229, 231, 238, 239, 239, 233, 231, 235, 240, 241, 241, 236, 235, 236, 241, 242, 242, 237, 236, 237, 242, 243, 243, 238, 237, 238, 243, 244, 244, 239, 238, 240, 245, 246, 246, 241, 240, 241, 246, 247, 247, 242, 241, 242, 247, 248, 248, 243, 242, 243, 248, 249, 249, 244, 243, 250, 251, 252, 252, 253, 250, 253, 252, 254, 254, 255, 253, 255, 254, 256, 256, 257, 255, 257, 256, 258, 258, 259, 257, 251, 260, 261, 261, 252, 251, 252, 261, 262, 262, 254, 252, 254, 262, 263, 263, 256, 254, 256, 263, 264, 264, 258, 256, 260, 265, 266, 266, 261, 260, 261, 266, 267, 267, 262, 261, 262, 267, 268, 268, 263, 262, 263, 268, 269, 269, 264, 263, 265, 270, 271, 271, 266, 265, 266, 271, 272, 272, 267, 266, 267, 272, 273, 273, 268, 267, 268, 273, 274, 274, 269, 268, 275, 276, 277, 277, 278, 275, 278, 277, 279, 279, 280, 278, 280, 279, 281, 281, 282, 280, 282, 281, 283, 283, 284, 282, 276, 285, 286, 286, 277, 276, 277, 286, 287, 287, 279, 277, 279, 287, 288, 288, 281, 279, 281, 288, 289, 289, 283, 281, 285, 290, 291, 291, 286, 285, 286, 291, 292, 292, 287, 286, 287, 292, 293, 293, 288, 287, 288, 293, 294, 294, 289, 288, 290, 295, 296, 296, 291, 290, 291, 296, 297, 297, 292, 291, 292, 297, 298, 298, 293, 292, 293, 298, 299, 299, 294, 293, 300, 301, 302, 302, 303, 300, 303, 302, 304, 304, 305, 303, 305, 304, 306, 306, 307, 305, 307, 306, 308, 308, 309, 307, 301, 310, 311, 311, 302, 301, 302, 311, 312, 312, 304, 302, 304, 312, 313, 313, 306, 304, 306, 313, 314, 314, 308, 306, 310, 315, 316, 316, 311, 310, 311, 316, 317, 317, 312, 311, 312, 317, 318, 318, 313, 312, 313, 318, 319, 319, 314, 313, 315, 320, 321, 321, 316, 315, 316, 321, 322, 322, 317, 316, 317, 322, 323, 323, 318, 317, 318, 323, 324, 324, 319, 318, 325, 326, 327, 327, 328, 325, 328, 327, 329, 329, 330, 328, 330, 329, 331, 331, 332, 330, 332, 331, 333, 333, 334, 332, 326, 335, 336, 336, 327, 326, 327, 336, 337, 337, 329, 327, 329, 337, 338, 338, 331, 329, 331, 338, 339, 339, 333, 331, 335, 340, 341, 341, 336, 335, 336, 341, 342, 342, 337, 336, 337, 342, 343, 343, 338, 337, 338, 343, 344, 344, 339, 338, 340, 345, 346, 346, 341, 340, 341, 346, 347, 347, 342, 341, 342, 347, 348, 348, 343, 342, 343, 348, 349, 349, 344, 343, 350, 351, 352, 352, 353, 350, 353, 352, 354, 354, 355, 353, 355, 354, 356, 356, 357, 355, 357, 356, 358, 358, 359, 357, 351, 360, 361, 361, 352, 351, 352, 361, 362, 362, 354, 352, 354, 362, 363, 363, 356, 354, 356, 363, 364, 364, 358, 356, 360, 365, 366, 366, 361, 360, 361, 366, 367, 367, 362, 361, 362, 367, 368, 368, 363, 362, 363, 368, 369, 369, 364, 363, 365, 370, 371, 371, 366, 365, 366, 371, 372, 372, 367, 366, 367, 372, 373, 373, 368, 367, 368, 373, 374, 374, 369, 368, 375, 376, 377, 377, 378, 375, 378, 377, 379, 379, 380, 378, 380, 379, 381, 381, 382, 380, 382, 381, 383, 383, 384, 382, 376, 385, 386, 386, 377, 376, 377, 386, 387, 387, 379, 377, 379, 387, 388, 388, 381, 379, 381, 388, 389, 389, 383, 381, 385, 390, 391, 391, 386, 385, 386, 391, 392, 392, 387, 386, 387, 392, 393, 393, 388, 387, 388, 393, 394, 394, 389, 388, 390, 395, 396, 396, 391, 390, 391, 396, 397, 397, 392, 391, 392, 397, 398, 398, 393, 392, 393, 398, 399, 399, 394, 393, 400, 401, 402, 402, 403, 400, 403, 402, 404, 404, 405, 403, 405, 404, 406, 406, 407, 405, 407, 406, 408, 408, 409, 407, 401, 410, 411, 411, 402, 401, 402, 411, 412, 412, 404, 402, 404, 412, 413, 413, 406, 404, 406, 413, 414, 414, 408, 406, 410, 415, 416, 416, 411, 410, 411, 416, 417, 417, 412, 411, 412, 417, 418, 418, 413, 412, 413, 418, 419, 419, 414, 413, 415, 420, 421, 421, 416, 415, 416, 421, 422, 422, 417, 416, 417, 422, 423, 423, 418, 417, 418, 423, 424, 424, 419, 418, 425, 426, 427, 427, 428, 425, 428, 427, 429, 429, 430, 428, 430, 429, 431, 431, 432, 430, 432, 431, 433, 433, 434, 432, 426, 435, 436, 436, 427, 426, 427, 436, 437, 437, 429, 427, 429, 437, 438, 438, 431, 429, 431, 438, 439, 439, 433, 431, 435, 440, 441, 441, 436, 435, 436, 441, 442, 442, 437, 436, 437, 442, 443, 443, 438, 437, 438, 443, 444, 444, 439, 438, 440, 445, 446, 446, 441, 440, 441, 446, 447, 447, 442, 441, 442, 447, 448, 448, 443, 442, 443, 448, 449, 449, 444, 443, 450, 451, 452, 452, 453, 450, 453, 452, 454, 454, 455, 453, 455, 454, 456, 456, 457, 455, 457, 456, 458, 458, 459, 457, 451, 460, 461, 461, 452, 451, 452, 461, 462, 462, 454, 452, 454, 462, 463, 463, 456, 454, 456, 463, 464, 464, 458, 456, 460, 465, 466, 466, 461, 460, 461, 466, 467, 467, 462, 461, 462, 467, 468, 468, 463, 462, 463, 468, 469, 469, 464, 463, 465, 470, 471, 471, 466, 465, 466, 471, 472, 472, 467, 466, 467, 472, 473, 473, 468, 467, 468, 473, 474, 474, 469, 468, 475, 476, 477, 477, 478, 475, 478, 477, 479, 479, 480, 478, 480, 479, 481, 481, 482, 480, 482, 481, 483, 483, 484, 482, 476, 485, 486, 486, 477, 476, 477, 486, 487, 487, 479, 477, 479, 487, 488, 488, 481, 479, 481, 488, 489, 489, 483, 481, 485, 490, 491, 491, 486, 485, 486, 491, 492, 492, 487, 486, 487, 492, 493, 493, 488, 487, 488, 493, 494, 494, 489, 488, 490, 495, 496, 496, 491, 490, 491, 496, 497, 497, 492, 491, 492, 497, 498, 498, 493, 492, 493, 498, 499, 499, 494, 493, 500, 501, 502, 502, 503, 500, 503, 502, 504, 504, 505, 503, 505, 504, 506, 506, 507, 505, 507, 506, 508, 508, 509, 507, 501, 510, 511, 511, 502, 501, 502, 511, 512, 512, 504, 502, 504, 512, 513, 513, 506, 504, 506, 513, 514, 514, 508, 506, 510, 515, 516, 516, 511, 510, 511, 516, 517, 517, 512, 511, 512, 517, 518, 518, 513, 512, 513, 518, 519, 519, 514, 513, 515, 520, 521, 521, 516, 515, 516, 521, 522, 522, 517, 516, 517, 522, 523, 523, 518, 517, 518, 523, 524, 524, 519, 518, 525, 526, 527, 527, 528, 525, 528, 527, 529, 529, 530, 528, 530, 529, 531, 531, 532, 530, 532, 531, 533, 533, 534, 532, 526, 535, 536, 536, 527, 526, 527, 536, 537, 537, 529, 527, 529, 537, 538, 538, 531, 529, 531, 538, 539, 539, 533, 531, 535, 540, 541, 541, 536, 535, 536, 541, 542, 542, 537, 536, 537, 542, 543, 543, 538, 537, 538, 543, 544, 544, 539, 538, 540, 545, 546, 546, 541, 540, 541, 546, 547, 547, 542, 541, 542, 547, 548, 548, 543, 542, 543, 548, 549, 549, 544, 543, 550, 551, 552, 552, 553, 550, 553, 552, 554, 554, 555, 553, 555, 554, 556, 556, 557, 555, 557, 556, 558, 558, 559, 557, 551, 560, 561, 561, 552, 551, 552, 561, 562, 562, 554, 552, 554, 562, 563, 563, 556, 554, 556, 563, 564, 564, 558, 556, 560, 565, 566, 566, 561, 560, 561, 566, 567, 567, 562, 561, 562, 567, 568, 568, 563, 562, 563, 568, 569, 569, 564, 563, 565, 570, 571, 571, 566, 565, 566, 571, 572, 572, 567, 566, 567, 572, 573, 573, 568, 567, 568, 573, 574, 574, 569, 568, 575, 576, 577, 577, 578, 575, 578, 577, 579, 579, 580, 578, 580, 579, 581, 581, 582, 580, 582, 581, 583, 583, 584, 582, 576, 585, 586, 586, 577, 576, 577, 586, 587, 587, 579, 577, 579, 587, 588, 588, 581, 579, 581, 588, 589, 589, 583, 581, 585, 590, 591, 591, 586, 585, 586, 591, 592, 592, 587, 586, 587, 592, 593, 593, 588, 587, 588, 593, 594, 594, 589, 588, 590, 595, 596, 596, 591, 590, 591, 596, 597, 597, 592, 591, 592, 597, 598, 598, 593, 592, 593, 598, 599, 599, 594, 593, 600, 601, 602, 602, 603, 600, 603, 602, 604, 604, 605, 603, 605, 604, 606, 606, 607, 605, 607, 606, 608, 608, 609, 607, 601, 610, 611, 611, 602, 601, 602, 611, 612, 612, 604, 602, 604, 612, 613, 613, 606, 604, 606, 613, 614, 614, 608, 606, 610, 615, 616, 616, 611, 610, 611, 616, 617, 617, 612, 611, 612, 617, 618, 618, 613, 612, 613, 618, 619, 619, 614, 613, 615, 620, 621, 621, 616, 615, 616, 621, 622, 622, 617, 616, 617, 622, 623, 623, 618, 617, 618, 623, 624, 624, 619, 618, 625, 626, 627, 627, 628, 625, 628, 627, 629, 629, 630, 628, 630, 629, 631, 631, 632, 630, 632, 631, 633, 633, 634, 632, 626, 635, 636, 636, 627, 626, 627, 636, 637, 637, 629, 627, 629, 637, 638, 638, 631, 629, 631, 638, 639, 639, 633, 631, 635, 640, 641, 641, 636, 635, 636, 641, 642, 642, 637, 636, 637, 642, 643, 643, 638, 637, 638, 643, 644, 644, 639, 638, 640, 645, 646, 646, 641, 640, 641, 646, 647, 647, 642, 641, 642, 647, 648, 648, 643, 642, 643, 648, 649, 649, 644, 643, 650, 651, 652, 652, 653, 650, 653, 652, 654, 654, 655, 653, 655, 654, 656, 656, 657, 655, 657, 656, 658, 658, 659, 657, 651, 660, 661, 661, 652, 651, 652, 661, 662, 662, 654, 652, 654, 662, 663, 663, 656, 654, 656, 663, 664, 664, 658, 656, 660, 665, 666, 666, 661, 660, 661, 666, 667, 667, 662, 661, 662, 667, 668, 668, 663, 662, 663, 668, 669, 669, 664, 663, 665, 670, 671, 671, 666, 665, 666, 671, 672, 672, 667, 666, 667, 672, 673, 673, 668, 667, 668, 673, 674, 674, 669, 668, 675, 676, 677, 677, 678, 675, 678, 677, 679, 679, 680, 678, 680, 679, 681, 681, 682, 680, 682, 681, 683, 683, 684, 682, 676, 685, 686, 686, 677, 676, 677, 686, 687, 687, 679, 677, 679, 687, 688, 688, 681, 679, 681, 688, 689, 689, 683, 681, 685, 690, 691, 691, 686, 685, 686, 691, 692, 692, 687, 686, 687, 692, 693, 693, 688, 687, 688, 693, 694, 694, 689, 688, 690, 695, 696, 696, 691, 690, 691, 696, 697, 697, 692, 691, 692, 697, 698, 698, 693, 692, 693, 698, 699, 699, 694, 693, 700, 701, 702, 702, 703, 700, 703, 702, 704, 704, 705, 703, 705, 704, 706, 706, 707, 705, 707, 706, 708, 708, 709, 707, 701, 710, 711, 711, 702, 701, 702, 711, 712, 712, 704, 702, 704, 712, 713, 713, 706, 704, 706, 713, 714, 714, 708, 706, 710, 715, 716, 716, 711, 710, 711, 716, 717, 717, 712, 711, 712, 717, 718, 718, 713, 712, 713, 718, 719, 719, 714, 713, 715, 720, 721, 721, 716, 715, 716, 721, 722, 722, 717, 716, 717, 722, 723, 723, 718, 717, 718, 723, 724, 724, 719, 718, 725, 726, 727, 727, 728, 725, 728, 727, 729, 729, 730, 728, 730, 729, 731, 731, 732, 730, 732, 731, 733, 733, 734, 732, 726, 735, 736, 736, 727, 726, 727, 736, 737, 737, 729, 727, 729, 737, 738, 738, 731, 729, 731, 738, 739, 739, 733, 731, 735, 740, 741, 741, 736, 735, 736, 741, 742, 742, 737, 736, 737, 742, 743, 743, 738, 737, 738, 743, 744, 744, 739, 738, 740, 745, 746, 746, 741, 740, 741, 746, 747, 747, 742, 741, 742, 747, 748, 748, 743, 742, 743, 748, 749, 749, 744, 743, 750, 751, 752, 752, 753, 750, 753, 752, 754, 754, 755, 753, 755, 754, 756, 756, 757, 755, 757, 756, 758, 758, 759, 757, 751, 760, 761, 761, 752, 751, 752, 761, 762, 762, 754, 752, 754, 762, 763, 763, 756, 754, 756, 763, 764, 764, 758, 756, 760, 765, 766, 766, 761, 760, 761, 766, 767, 767, 762, 761, 762, 767, 768, 768, 763, 762, 763, 768, 769, 769, 764, 763, 765, 770, 771, 771, 766, 765, 766, 771, 772, 772, 767, 766, 767, 772, 773, 773, 768, 767, 768, 773, 774, 774, 769, 768, 775, 776, 777, 777, 778, 775, 778, 777, 779, 779, 780, 778, 780, 779, 781, 781, 782, 780, 782, 781, 783, 783, 784, 782, 776, 785, 786, 786, 777, 776, 777, 786, 787, 787, 779, 777, 779, 787, 788, 788, 781, 779, 781, 788, 789, 789, 783, 781, 785, 790, 791, 791, 786, 785, 786, 791, 792, 792, 787, 786, 787, 792, 793, 793, 788, 787, 788, 793, 794, 794, 789, 788, 790, 795, 796, 796, 791, 790, 791, 796, 797, 797, 792, 791, 792, 797, 798, 798, 793, 792, 793, 798, 799, 799, 794, 793
  };
  core->setElementIndexData(indices, sizeof(indices), GL_STATIC_DRAW);

  return core;
}


GeometryCoreSP GeometryCoreFactory::createTeapotFlat(GLfloat size) {
  // create geometry core
  auto core = GeometryCore::create(GL_TRIANGLES, DrawMode::ARRAYS);

  // define vertices (800 vertices)
  GLfloat vertices[] = {
      17.83489990234375, 0, 30.573999404907227, 16.452699661254883, -7.000179767608643, 30.573999404907227, 16.223100662231445, -6.902520179748535, 31.51460075378418, 17.586000442504883, 0, 31.51460075378418, 16.48940086364746, -7.015810012817383, 31.828100204467773, 17.87470054626465, 0, 31.828100204467773, 17.031099319458008, -7.246280193328857, 31.51460075378418, 18.46190071105957, 0, 31.51460075378418, 17.62779998779297, -7.500199794769287, 30.573999404907227, 19.108800888061523, 0, 30.573999404907227, 12.662699699401855, -12.662699699401855, 30.573999404907227, 12.486100196838379, -12.486100196838379, 31.51460075378418, 12.690999984741211, -12.690999984741211, 31.828100204467773, 13.10789966583252, -13.10789966583252, 31.51460075378418, 13.56719970703125, -13.56719970703125, 30.573999404907227, 7.000179767608643, -16.452699661254883, 30.573999404907227, 6.902520179748535, -16.223100662231445, 31.51460075378418, 7.015810012817383, -16.48940086364746, 31.828100204467773, 7.246280193328857, -17.031099319458008, 31.51460075378418, 7.500199794769287, -17.62779998779297, 30.573999404907227, 0, -17.83489990234375, 30.573999404907227, 0, -17.586000442504883, 31.51460075378418, 0, -17.87470054626465, 31.828100204467773, 0, -18.46190071105957, 31.51460075378418, 0, -19.108800888061523, 30.573999404907227, 0, -17.83489990234375, 30.573999404907227, -7.483870029449463, -16.452699661254883, 30.573999404907227, -7.106579780578613, -16.223100662231445, 31.51460075378418, 0, -17.586000442504883, 31.51460075378418, -7.07627010345459, -16.48940086364746, 31.828100204467773, 0, -17.87470054626465, 31.828100204467773, -7.25383996963501, -17.031099319458008, 31.51460075378418, 0, -18.46190071105957, 31.51460075378418, -7.500199794769287, -17.62779998779297, 30.573999404907227, 0, -19.108800888061523, 30.573999404907227, -13.092700004577637, -12.662699699401855, 30.573999404907227, -12.667499542236328, -12.486100196838379, 31.51460075378418, -12.744799613952637, -12.690999984741211, 31.828100204467773, -13.11460018157959, -13.10789966583252, 31.51460075378418, -13.56719970703125, -13.56719970703125, 30.573999404907227, -16.61389923095703, -7.000179767608643, 30.573999404907227, -16.291099548339844, -6.902520179748535, 31.51460075378418, -16.50950050354004, -7.015810012817383, 31.828100204467773, -17.033599853515625, -7.246280193328857, 31.51460075378418, -17.62779998779297, -7.500199794769287, 30.573999404907227, -17.83489990234375, 0, 30.573999404907227, -17.586000442504883, 0, 31.51460075378418, -17.87470054626465, 0, 31.828100204467773, -18.46190071105957, 0, 31.51460075378418, -19.108800888061523, 0, 30.573999404907227, -17.83489990234375, 0, 30.573999404907227, -16.452699661254883, 7.000179767608643, 30.573999404907227, -16.223100662231445, 6.902520179748535, 31.51460075378418, -17.586000442504883, 0, 31.51460075378418, -16.48940086364746, 7.015810012817383, 31.828100204467773, -17.87470054626465, 0, 31.828100204467773, -17.031099319458008, 7.246280193328857, 31.51460075378418, -18.46190071105957, 0, 31.51460075378418, -17.62779998779297, 7.500199794769287, 30.573999404907227, -19.108800888061523, 0, 30.573999404907227, -12.662699699401855, 12.662699699401855, 30.573999404907227, -12.486100196838379, 12.486100196838379, 31.51460075378418, -12.690999984741211, 12.690999984741211, 31.828100204467773, -13.10789966583252, 13.10789966583252, 31.51460075378418, -13.56719970703125, 13.56719970703125, 30.573999404907227, -7.000179767608643, 16.452699661254883, 30.573999404907227, -6.902520179748535, 16.223100662231445, 31.51460075378418, -7.015810012817383, 16.48940086364746, 31.828100204467773, -7.246280193328857, 17.031099319458008, 31.51460075378418, -7.500199794769287, 17.62779998779297, 30.573999404907227, 0, 17.83489990234375, 30.573999404907227, 0, 17.586000442504883, 31.51460075378418, 0, 17.87470054626465, 31.828100204467773, 0, 18.46190071105957, 31.51460075378418, 0, 19.108800888061523, 30.573999404907227, 0, 17.83489990234375, 30.573999404907227, 7.000179767608643, 16.452699661254883, 30.573999404907227, 6.902520179748535, 16.223100662231445, 31.51460075378418, 0, 17.586000442504883, 31.51460075378418, 7.015810012817383, 16.48940086364746, 31.828100204467773, 0, 17.87470054626465, 31.828100204467773, 7.246280193328857, 17.031099319458008, 31.51460075378418, 0, 18.46190071105957, 31.51460075378418, 7.500199794769287, 17.62779998779297, 30.573999404907227, 0, 19.108800888061523, 30.573999404907227, 12.662699699401855, 12.662699699401855, 30.573999404907227, 12.486100196838379, 12.486100196838379, 31.51460075378418, 12.690999984741211, 12.690999984741211, 31.828100204467773, 13.10789966583252, 13.10789966583252, 31.51460075378418, 13.56719970703125, 13.56719970703125, 30.573999404907227, 16.452699661254883, 7.000179767608643, 30.573999404907227, 16.223100662231445, 6.902520179748535, 31.51460075378418, 16.48940086364746, 7.015810012817383, 31.828100204467773, 17.031099319458008, 7.246280193328857, 31.51460075378418, 17.62779998779297, 7.500199794769287, 30.573999404907227, 17.83489990234375, 0, 30.573999404907227, 17.586000442504883, 0, 31.51460075378418, 17.87470054626465, 0, 31.828100204467773, 18.46190071105957, 0, 31.51460075378418, 19.108800888061523, 0, 30.573999404907227, 19.108800888061523, 0, 30.573999404907227, 17.62779998779297, -7.500199794769287, 30.573999404907227, 19.785400390625, -8.418190002441406, 25.572900772094727, 21.447599411010742, 0, 25.572900772094727, 21.667600631713867, -9.218990325927734, 20.661399841308594, 23.487899780273438, 0, 20.661399841308594, 22.99880027770996, -9.785409927368164, 15.928999900817871, 24.930999755859375, 0, 15.928999900817871, 23.503799438476562, -10.000300407409668, 11.465299606323242, 25.4783992767334, 0, 11.465299606323242, 13.56719970703125, -13.56719970703125, 30.573999404907227, 15.227800369262695, -15.227800369262695, 25.572900772094727, 16.67639923095703, -16.67639923095703, 20.661399841308594, 17.701000213623047, -17.701000213623047, 15.928999900817871, 18.089599609375, -18.089599609375, 11.465299606323242, 7.500199794769287, -17.62779998779297, 30.573999404907227, 8.418190002441406, -19.785400390625, 25.572900772094727, 9.218990325927734, -21.667600631713867, 20.661399841308594, 9.785409927368164, -22.99880027770996, 15.928999900817871, 10.000300407409668, -23.503799438476562, 11.465299606323242, 0, -19.108800888061523, 30.573999404907227, 0, -21.447599411010742, 25.572900772094727, 0, -23.487899780273438, 20.661399841308594, 0, -24.930999755859375, 15.928999900817871, 0, -25.4783992767334, 11.465299606323242, 0, -19.108800888061523, 30.573999404907227, -7.500199794769287, -17.62779998779297, 30.573999404907227, -8.418190002441406, -19.785400390625, 25.572900772094727, 0, -21.447599411010742, 25.572900772094727, -9.218990325927734, -21.667600631713867, 20.661399841308594, 0, -23.487899780273438, 20.661399841308594, -9.785409927368164, -22.99880027770996, 15.928999900817871, 0, -24.930999755859375, 15.928999900817871, -10.000300407409668, -23.503799438476562, 11.465299606323242, 0, -25.4783992767334, 11.465299606323242, -13.56719970703125, -13.56719970703125, 30.573999404907227, -15.227800369262695, -15.227800369262695, 25.572900772094727, -16.67639923095703, -16.67639923095703, 20.661399841308594, -17.701000213623047, -17.701000213623047, 15.928999900817871, -18.089599609375, -18.089599609375, 11.465299606323242, -17.62779998779297, -7.500199794769287, 30.573999404907227, -19.785400390625, -8.418190002441406, 25.572900772094727, -21.667600631713867, -9.218990325927734, 20.661399841308594, -22.99880027770996, -9.785409927368164, 15.928999900817871, -23.503799438476562, -10.000300407409668, 11.465299606323242, -19.108800888061523, 0, 30.573999404907227, -21.447599411010742, 0, 25.572900772094727, -23.487899780273438, 0, 20.661399841308594, -24.930999755859375, 0, 15.928999900817871, -25.4783992767334, 0, 11.465299606323242, -19.108800888061523, 0, 30.573999404907227, -17.62779998779297, 7.500199794769287, 30.573999404907227, -19.785400390625, 8.418190002441406, 25.572900772094727, -21.447599411010742, 0, 25.572900772094727, -21.667600631713867, 9.218990325927734, 20.661399841308594, -23.487899780273438, 0, 20.661399841308594, -22.99880027770996, 9.785409927368164, 15.928999900817871, -24.930999755859375, 0, 15.928999900817871, -23.503799438476562, 10.000300407409668, 11.465299606323242, -25.4783992767334, 0, 11.465299606323242, -13.56719970703125, 13.56719970703125, 30.573999404907227, -15.227800369262695, 15.227800369262695, 25.572900772094727, -16.67639923095703, 16.67639923095703, 20.661399841308594, -17.701000213623047, 17.701000213623047, 15.928999900817871, -18.089599609375, 18.089599609375, 11.465299606323242, -7.500199794769287, 17.62779998779297, 30.573999404907227, -8.418190002441406, 19.785400390625, 25.572900772094727, -9.218990325927734, 21.667600631713867, 20.661399841308594, -9.785409927368164, 22.99880027770996, 15.928999900817871, -10.000300407409668, 23.503799438476562, 11.465299606323242, 0, 19.108800888061523, 30.573999404907227, 0, 21.447599411010742, 25.572900772094727, 0, 23.487899780273438, 20.661399841308594, 0, 24.930999755859375, 15.928999900817871, 0, 25.4783992767334, 11.465299606323242, 0, 19.108800888061523, 30.573999404907227, 7.500199794769287, 17.62779998779297, 30.573999404907227, 8.418190002441406, 19.785400390625, 25.572900772094727, 0, 21.447599411010742, 25.572900772094727, 9.218990325927734, 21.667600631713867, 20.661399841308594, 0, 23.487899780273438, 20.661399841308594, 9.785409927368164, 22.99880027770996, 15.928999900817871, 0, 24.930999755859375, 15.928999900817871, 10.000300407409668, 23.503799438476562, 11.465299606323242, 0, 25.4783992767334, 11.465299606323242, 13.56719970703125, 13.56719970703125, 30.573999404907227, 15.227800369262695, 15.227800369262695, 25.572900772094727, 16.67639923095703, 16.67639923095703, 20.661399841308594, 17.701000213623047, 17.701000213623047, 15.928999900817871, 18.089599609375, 18.089599609375, 11.465299606323242, 17.62779998779297, 7.500199794769287, 30.573999404907227, 19.785400390625, 8.418190002441406, 25.572900772094727, 21.667600631713867, 9.218990325927734, 20.661399841308594, 22.99880027770996, 9.785409927368164, 15.928999900817871, 23.503799438476562, 10.000300407409668, 11.465299606323242, 19.108800888061523, 0, 30.573999404907227, 21.447599411010742, 0, 25.572900772094727, 23.487899780273438, 0, 20.661399841308594, 24.930999755859375, 0, 15.928999900817871, 25.4783992767334, 0, 11.465299606323242, 25.4783992767334, 0, 11.465299606323242, 23.503799438476562, -10.000300407409668, 11.465299606323242, 22.5856990814209, -9.609620094299316, 7.688300132751465, 24.48310089111328, 0, 7.688300132751465, 20.565799713134766, -8.750229835510254, 4.89661979675293, 22.29360008239746, 0, 4.89661979675293, 18.54599952697754, -7.890830039978027, 3.0006699562072754, 20.104000091552734, 0, 3.0006699562072754, 17.62779998779297, -7.500199794769287, 1.9108799695968628, 19.108800888061523, 0, 1.9108799695968628, 18.089599609375, -18.089599609375, 11.465299606323242, 17.382999420166016, -17.382999420166016, 7.688300132751465, 15.828399658203125, -15.828399658203125, 4.89661979675293, 14.273900032043457, -14.273900032043457, 3.0006699562072754, 13.56719970703125, -13.56719970703125, 1.9108799695968628, 10.000300407409668, -23.503799438476562, 11.465299606323242, 9.609620094299316, -22.5856990814209, 7.688300132751465, 8.750229835510254, -20.565799713134766, 4.89661979675293, 7.890830039978027, -18.54599952697754, 3.0006699562072754, 7.500199794769287, -17.62779998779297, 1.9108799695968628, 0, -25.4783992767334, 11.465299606323242, 0, -24.48310089111328, 7.688300132751465, 0, -22.29360008239746, 4.89661979675293, 0, -20.104000091552734, 3.0006699562072754, 0, -19.108800888061523, 1.9108799695968628, 0, -25.4783992767334, 11.465299606323242, -10.000300407409668, -23.503799438476562, 11.465299606323242, -9.609620094299316, -22.5856990814209, 7.688300132751465, 0, -24.48310089111328, 7.688300132751465, -8.750229835510254, -20.565799713134766, 4.89661979675293, 0, -22.29360008239746, 4.89661979675293, -7.890830039978027, -18.54599952697754, 3.0006699562072754, 0, -20.104000091552734, 3.0006699562072754, -7.500199794769287, -17.62779998779297, 1.9108799695968628, 0, -19.108800888061523, 1.9108799695968628, -18.089599609375, -18.089599609375, 11.465299606323242, -17.382999420166016, -17.382999420166016, 7.688300132751465, -15.828399658203125, -15.828399658203125, 4.89661979675293, -14.273900032043457, -14.273900032043457, 3.0006699562072754, -13.56719970703125, -13.56719970703125, 1.9108799695968628, -23.503799438476562, -10.000300407409668, 11.465299606323242, -22.5856990814209, -9.609620094299316, 7.688300132751465, -20.565799713134766, -8.750229835510254, 4.89661979675293, -18.54599952697754, -7.890830039978027, 3.0006699562072754, -17.62779998779297, -7.500199794769287, 1.9108799695968628, -25.4783992767334, 0, 11.465299606323242, -24.48310089111328, 0, 7.688300132751465, -22.29360008239746, 0, 4.89661979675293, -20.104000091552734, 0, 3.0006699562072754, -19.108800888061523, 0, 1.9108799695968628, -25.4783992767334, 0, 11.465299606323242, -23.503799438476562, 10.000300407409668, 11.465299606323242, -22.5856990814209, 9.609620094299316, 7.688300132751465, -24.48310089111328, 0, 7.688300132751465, -20.565799713134766, 8.750229835510254, 4.89661979675293, -22.29360008239746, 0, 4.89661979675293, -18.54599952697754, 7.890830039978027, 3.0006699562072754, -20.104000091552734, 0, 3.0006699562072754, -17.62779998779297, 7.500199794769287, 1.9108799695968628, -19.108800888061523, 0, 1.9108799695968628, -18.089599609375, 18.089599609375, 11.465299606323242, -17.382999420166016, 17.382999420166016, 7.688300132751465, -15.828399658203125, 15.828399658203125, 4.89661979675293, -14.273900032043457, 14.273900032043457, 3.0006699562072754, -13.56719970703125, 13.56719970703125, 1.9108799695968628, -10.000300407409668, 23.503799438476562, 11.465299606323242, -9.609620094299316, 22.5856990814209, 7.688300132751465, -8.750229835510254, 20.565799713134766, 4.89661979675293, -7.890830039978027, 18.54599952697754, 3.0006699562072754, -7.500199794769287, 17.62779998779297, 1.9108799695968628, 0, 25.4783992767334, 11.465299606323242, 0, 24.48310089111328, 7.688300132751465, 0, 22.29360008239746, 4.89661979675293, 0, 20.104000091552734, 3.0006699562072754, 0, 19.108800888061523, 1.9108799695968628, 0, 25.4783992767334, 11.465299606323242, 10.000300407409668, 23.503799438476562, 11.465299606323242, 9.609620094299316, 22.5856990814209, 7.688300132751465, 0, 24.48310089111328, 7.688300132751465, 8.750229835510254, 20.565799713134766, 4.89661979675293, 0, 22.29360008239746, 4.89661979675293, 7.890830039978027, 18.54599952697754, 3.0006699562072754, 0, 20.104000091552734, 3.0006699562072754, 7.500199794769287, 17.62779998779297, 1.9108799695968628, 0, 19.108800888061523, 1.9108799695968628, 18.089599609375, 18.089599609375, 11.465299606323242, 17.382999420166016, 17.382999420166016, 7.688300132751465, 15.828399658203125, 15.828399658203125, 4.89661979675293, 14.273900032043457, 14.273900032043457, 3.0006699562072754, 13.56719970703125, 13.56719970703125, 1.9108799695968628, 23.503799438476562, 10.000300407409668, 11.465299606323242, 22.5856990814209, 9.609620094299316, 7.688300132751465, 20.565799713134766, 8.750229835510254, 4.89661979675293, 18.54599952697754, 7.890830039978027, 3.0006699562072754, 17.62779998779297, 7.500199794769287, 1.9108799695968628, 25.4783992767334, 0, 11.465299606323242, 24.48310089111328, 0, 7.688300132751465, 22.29360008239746, 0, 4.89661979675293, 20.104000091552734, 0, 3.0006699562072754, 19.108800888061523, 0, 1.9108799695968628, 19.108800888061523, 0, 1.9108799695968628, 17.62779998779297, -7.500199794769287, 1.9108799695968628, 17.228500366210938, -7.330269813537598, 1.2092299461364746, 18.675800323486328, 0, 1.2092299461364746, 15.093799591064453, -6.422039985656738, 0.5971490144729614, 16.361900329589844, 0, 0.5971490144729614, 9.819259643554688, -4.177840232849121, 0.16421599686145782, 10.644200325012207, 0, 0.16421599686145782, 0, 0, 0, 0, 0, 0, 13.56719970703125, -13.56719970703125, 1.9108799695968628, 13.25979995727539, -13.25979995727539, 1.2092299461364746, 11.616900444030762, -11.616900444030762, 0.5971490144729614, 7.557370185852051, -7.557370185852051, 0.16421599686145782, 0, 0, 0, 7.500199794769287, -17.62779998779297, 1.9108799695968628, 7.330269813537598, -17.228500366210938, 1.2092299461364746, 6.422039985656738, -15.093799591064453, 0.5971490144729614, 4.177840232849121, -9.819259643554688, 0.16421599686145782, 0, 0, 0, 0, -19.108800888061523, 1.9108799695968628, 0, -18.675800323486328, 1.2092299461364746, 0, -16.361900329589844, 0.5971490144729614, 0, -10.644200325012207, 0.16421599686145782, 0, 0, 0, 0, -19.108800888061523, 1.9108799695968628, -7.500199794769287, -17.62779998779297, 1.9108799695968628, -7.330269813537598, -17.228500366210938, 1.2092299461364746, 0, -18.675800323486328, 1.2092299461364746, -6.422039985656738, -15.093799591064453, 0.5971490144729614, 0, -16.361900329589844, 0.5971490144729614, -4.177840232849121, -9.819259643554688, 0.16421599686145782, 0, -10.644200325012207, 0.16421599686145782, 0, 0, 0, 0, 0, 0, -13.56719970703125, -13.56719970703125, 1.9108799695968628, -13.25979995727539, -13.25979995727539, 1.2092299461364746, -11.616900444030762, -11.616900444030762, 0.5971490144729614, -7.557370185852051, -7.557370185852051, 0.16421599686145782, 0, 0, 0, -17.62779998779297, -7.500199794769287, 1.9108799695968628, -17.228500366210938, -7.330269813537598, 1.2092299461364746, -15.093799591064453, -6.422039985656738, 0.5971490144729614, -9.819259643554688, -4.177840232849121, 0.16421599686145782, 0, 0, 0, -19.108800888061523, 0, 1.9108799695968628, -18.675800323486328, 0, 1.2092299461364746, -16.361900329589844, 0, 0.5971490144729614, -10.644200325012207, 0, 0.16421599686145782, 0, 0, 0, -19.108800888061523, 0, 1.9108799695968628, -17.62779998779297, 7.500199794769287, 1.9108799695968628, -17.228500366210938, 7.330269813537598, 1.2092299461364746, -18.675800323486328, 0, 1.2092299461364746, -15.093799591064453, 6.422039985656738, 0.5971490144729614, -16.361900329589844, 0, 0.5971490144729614, -9.819259643554688, 4.177840232849121, 0.16421599686145782, -10.644200325012207, 0, 0.16421599686145782, 0, 0, 0, 0, 0, 0, -13.56719970703125, 13.56719970703125, 1.9108799695968628, -13.25979995727539, 13.25979995727539, 1.2092299461364746, -11.616900444030762, 11.616900444030762, 0.5971490144729614, -7.557370185852051, 7.557370185852051, 0.16421599686145782, 0, 0, 0, -7.500199794769287, 17.62779998779297, 1.9108799695968628, -7.330269813537598, 17.228500366210938, 1.2092299461364746, -6.422039985656738, 15.093799591064453, 0.5971490144729614, -4.177840232849121, 9.819259643554688, 0.16421599686145782, 0, 0, 0, 0, 19.108800888061523, 1.9108799695968628, 0, 18.675800323486328, 1.2092299461364746, 0, 16.361900329589844, 0.5971490144729614, 0, 10.644200325012207, 0.16421599686145782, 0, 0, 0, 0, 19.108800888061523, 1.9108799695968628, 7.500199794769287, 17.62779998779297, 1.9108799695968628, 7.330269813537598, 17.228500366210938, 1.2092299461364746, 0, 18.675800323486328, 1.2092299461364746, 6.422039985656738, 15.093799591064453, 0.5971490144729614, 0, 16.361900329589844, 0.5971490144729614, 4.177840232849121, 9.819259643554688, 0.16421599686145782, 0, 10.644200325012207, 0.16421599686145782, 0, 0, 0, 0, 0, 0, 13.56719970703125, 13.56719970703125, 1.9108799695968628, 13.25979995727539, 13.25979995727539, 1.2092299461364746, 11.616900444030762, 11.616900444030762, 0.5971490144729614, 7.557370185852051, 7.557370185852051, 0.16421599686145782, 0, 0, 0, 17.62779998779297, 7.500199794769287, 1.9108799695968628, 17.228500366210938, 7.330269813537598, 1.2092299461364746, 15.093799591064453, 6.422039985656738, 0.5971490144729614, 9.819259643554688, 4.177840232849121, 0.16421599686145782, 0, 0, 0, 19.108800888061523, 0, 1.9108799695968628, 18.675800323486328, 0, 1.2092299461364746, 16.361900329589844, 0, 0.5971490144729614, 10.644200325012207, 0, 0.16421599686145782, 0, 0, 0, -20.382699966430664, 0, 25.796899795532227, -20.1835994720459, -2.149739980697632, 26.244699478149414, -26.511600494384766, -2.149739980697632, 26.192899703979492, -26.334299087524414, 0, 25.752099990844727, -31.156299591064453, -2.149739980697632, 25.830400466918945, -30.733299255371094, 0, 25.438600540161133, -34.016998291015625, -2.149739980697632, 24.846500396728516, -33.46030044555664, 0, 24.587600708007812, -34.99290084838867, -2.149739980697632, 22.930500030517578, -34.39580154418945, 0, 22.930500030517578, -19.74570083618164, -2.8663198947906494, 27.229999542236328, -26.901599884033203, -2.8663198947906494, 27.162799835205078, -32.08679962158203, -2.8663198947906494, 26.69260025024414, -35.241798400878906, -2.8663198947906494, 25.416200637817383, -36.30670166015625, -2.8663198947906494, 22.930500030517578, -19.30780029296875, -2.149739980697632, 28.215299606323242, -27.29159927368164, -2.149739980697632, 28.132699966430664, -33.017398834228516, -2.149739980697632, 27.55470085144043, -36.46649932861328, -2.149739980697632, 25.98579978942871, -37.620399475097656, -2.149739980697632, 22.930500030517578, -19.108800888061523, 0, 28.66320037841797, -27.468900680541992, 0, 28.57360076904297, -33.440399169921875, 0, 27.94659996032715, -37.02330017089844, 0, 26.244699478149414, -38.21760177612305, 0, 22.930500030517578, -19.108800888061523, 0, 28.66320037841797, -19.30780029296875, 2.149739980697632, 28.215299606323242, -27.29159927368164, 2.149739980697632, 28.132699966430664, -27.468900680541992, 0, 28.57360076904297, -33.017398834228516, 2.149739980697632, 27.55470085144043, -33.440399169921875, 0, 27.94659996032715, -36.46649932861328, 2.149739980697632, 25.98579978942871, -37.02330017089844, 0, 26.244699478149414, -37.620399475097656, 2.149739980697632, 22.930500030517578, -38.21760177612305, 0, 22.930500030517578, -19.74570083618164, 2.8663198947906494, 27.229999542236328, -26.901599884033203, 2.8663198947906494, 27.162799835205078, -32.08679962158203, 2.8663198947906494, 26.69260025024414, -35.241798400878906, 2.8663198947906494, 25.416200637817383, -36.30670166015625, 2.8663198947906494, 22.930500030517578, -20.1835994720459, 2.149739980697632, 26.244699478149414, -26.511600494384766, 2.149739980697632, 26.192899703979492, -31.156299591064453, 2.149739980697632, 25.830400466918945, -34.016998291015625, 2.149739980697632, 24.846500396728516, -34.99290084838867, 2.149739980697632, 22.930500030517578, -20.382699966430664, 0, 25.796899795532227, -26.334299087524414, 0, 25.752099990844727, -30.733299255371094, 0, 25.438600540161133, -33.46030044555664, 0, 24.587600708007812, -34.39580154418945, 0, 22.930500030517578, -34.39580154418945, 0, 22.930500030517578, -34.99290084838867, -2.149739980697632, 22.930500030517578, -34.44089889526367, -2.149739980697632, 20.082199096679688, -33.89820098876953, 0, 20.33289909362793, -32.711299896240234, -2.149739980697632, 16.81529998779297, -32.32569885253906, 0, 17.197900772094727, -29.69420051574707, -2.149739980697632, 13.590499877929688, -29.558900833129883, 0, 14.062899589538574, -25.279300689697266, -2.149739980697632, 10.8681001663208, -25.4783992767334, 0, 11.465299606323242, -36.30670166015625, -2.8663198947906494, 22.930500030517578, -35.6348991394043, -2.8663198947906494, 19.530500411987305, -33.55979919433594, -2.8663198947906494, 15.973699569702148, -29.99180030822754, -2.8663198947906494, 12.551300048828125, -24.841400146484375, -2.8663198947906494, 9.554389953613281, -37.620399475097656, -2.149739980697632, 22.930500030517578, -36.82889938354492, -2.149739980697632, 18.97879981994629, -34.408199310302734, -2.149739980697632, 15.132100105285645, -30.289499282836914, -2.149739980697632, 11.512200355529785, -24.403499603271484, -2.149739980697632, 8.240659713745117, -38.21760177612305, 0, 22.930500030517578, -37.37160110473633, 0, 18.728099822998047, -34.79389953613281, 0, 14.749600410461426, -30.424800872802734, 0, 11.039799690246582, -24.204500198364258, 0, 7.643509864807129, -38.21760177612305, 0, 22.930500030517578, -37.620399475097656, 2.149739980697632, 22.930500030517578, -36.82889938354492, 2.149739980697632, 18.97879981994629, -37.37160110473633, 0, 18.728099822998047, -34.408199310302734, 2.149739980697632, 15.132100105285645, -34.79389953613281, 0, 14.749600410461426, -30.289499282836914, 2.149739980697632, 11.512200355529785, -30.424800872802734, 0, 11.039799690246582, -24.403499603271484, 2.149739980697632, 8.240659713745117, -24.204500198364258, 0, 7.643509864807129, -36.30670166015625, 2.8663198947906494, 22.930500030517578, -35.6348991394043, 2.8663198947906494, 19.530500411987305, -33.55979919433594, 2.8663198947906494, 15.973699569702148, -29.99180030822754, 2.8663198947906494, 12.551300048828125, -24.841400146484375, 2.8663198947906494, 9.554389953613281, -34.99290084838867, 2.149739980697632, 22.930500030517578, -34.44089889526367, 2.149739980697632, 20.082199096679688, -32.711299896240234, 2.149739980697632, 16.81529998779297, -29.69420051574707, 2.149739980697632, 13.590499877929688, -25.279300689697266, 2.149739980697632, 10.8681001663208, -34.39580154418945, 0, 22.930500030517578, -33.89820098876953, 0, 20.33289909362793, -32.32569885253906, 0, 17.197900772094727, -29.558900833129883, 0, 14.062899589538574, -25.4783992767334, 0, 11.465299606323242, 21.656600952148438, 0, 18.15329933166504, 21.656600952148438, -4.729420185089111, 16.511199951171875, 28.233999252319336, -4.270359992980957, 18.339000701904297, 27.76740074157715, 0, 19.55660057067871, 31.011899948120117, -3.2604401111602783, 22.221399307250977, 30.4148006439209, 0, 22.930500030517578, 32.59560012817383, -2.2505099773406982, 26.764400482177734, 31.867900848388672, 0, 27.020999908447266, 35.5900993347168, -1.791450023651123, 30.573999404907227, 34.39580154418945, 0, 30.573999404907227, 21.656600952148438, -6.3059000968933105, 12.89840030670166, 29.260299682617188, -5.693819999694824, 15.660200119018555, 32.32569885253906, -4.347249984741211, 20.661399841308594, 34.19670104980469, -3.0006699562072754, 26.199899673461914, 38.21760177612305, -2.3886001110076904, 30.573999404907227, 21.656600952148438, -4.729420185089111, 9.285670280456543, 30.286699295043945, -4.270359992980957, 12.981499671936035, 33.639400482177734, -3.2604401111602783, 19.101299285888672, 35.79790115356445, -2.2505099773406982, 25.635400772094727, 40.845001220703125, -1.791450023651123, 30.573999404907227, 21.656600952148438, 0, 7.643509864807129, 30.75320053100586, 0, 11.763799667358398, 34.23659896850586, 0, 18.392200469970703, 36.52560043334961, 0, 25.378799438476562, 42.03929901123047, 0, 30.573999404907227, 21.656600952148438, 0, 7.643509864807129, 21.656600952148438, 4.729420185089111, 9.285670280456543, 30.286699295043945, 4.270359992980957, 12.981499671936035, 30.75320053100586, 0, 11.763799667358398, 33.639400482177734, 3.2604401111602783, 19.101299285888672, 34.23659896850586, 0, 18.392200469970703, 35.79790115356445, 2.2505099773406982, 25.635400772094727, 36.52560043334961, 0, 25.378799438476562, 40.845001220703125, 1.791450023651123, 30.573999404907227, 42.03929901123047, 0, 30.573999404907227, 21.656600952148438, 6.3059000968933105, 12.89840030670166, 29.260299682617188, 5.693819999694824, 15.660200119018555, 32.32569885253906, 4.347249984741211, 20.661399841308594, 34.19670104980469, 3.0006699562072754, 26.199899673461914, 38.21760177612305, 2.3886001110076904, 30.573999404907227, 21.656600952148438, 4.729420185089111, 16.511199951171875, 28.233999252319336, 4.270359992980957, 18.339000701904297, 31.011899948120117, 3.2604401111602783, 22.221399307250977, 32.59560012817383, 2.2505099773406982, 26.764400482177734, 35.5900993347168, 1.791450023651123, 30.573999404907227, 21.656600952148438, 0, 18.15329933166504, 27.76740074157715, 0, 19.55660057067871, 30.4148006439209, 0, 22.930500030517578, 31.867900848388672, 0, 27.020999908447266, 34.39580154418945, 0, 30.573999404907227, 34.39580154418945, 0, 30.573999404907227, 35.5900993347168, -1.791450023651123, 30.573999404907227, 36.59049987792969, -1.679479956626892, 31.137699127197266, 35.3114013671875, 0, 31.111499786376953, 37.18870162963867, -1.4331599473953247, 31.332599639892578, 35.98820114135742, 0, 31.290599822998047, 37.206600189208984, -1.1868300437927246, 31.1481990814209, 36.187198638916016, 0, 31.111499786376953, 36.46590042114258, -1.074869990348816, 30.573999404907227, 35.669700622558594, 0, 30.573999404907227, 38.21760177612305, -2.3886001110076904, 30.573999404907227, 39.40439987182617, -2.2393100261688232, 31.195499420166016, 39.829898834228516, -1.9108799695968628, 31.424999237060547, 39.44919967651367, -1.582450032234192, 31.229000091552734, 38.21760177612305, -1.4331599473953247, 30.573999404907227, 40.845001220703125, -1.791450023651123, 30.573999404907227, 42.218299865722656, -1.679479956626892, 31.25320053100586, 42.47100067138672, -1.4331599473953247, 31.51740074157715, 41.69169998168945, -1.1868300437927246, 31.309900283813477, 39.969200134277344, -1.074869990348816, 30.573999404907227, 42.03929901123047, 0, 30.573999404907227, 43.49729919433594, 0, 31.279399871826172, 43.67150115966797, 0, 31.55929946899414, 42.71110153198242, 0, 31.346599578857422, 40.76539993286133, 0, 30.573999404907227, 42.03929901123047, 0, 30.573999404907227, 40.845001220703125, 1.791450023651123, 30.573999404907227, 42.218299865722656, 1.679479956626892, 31.25320053100586, 43.49729919433594, 0, 31.279399871826172, 42.47100067138672, 1.4331599473953247, 31.51740074157715, 43.67150115966797, 0, 31.55929946899414, 41.69169998168945, 1.1868300437927246, 31.309900283813477, 42.71110153198242, 0, 31.346599578857422, 39.969200134277344, 1.074869990348816, 30.573999404907227, 40.76539993286133, 0, 30.573999404907227, 38.21760177612305, 2.3886001110076904, 30.573999404907227, 39.40439987182617, 2.2393100261688232, 31.195499420166016, 39.829898834228516, 1.9108799695968628, 31.424999237060547, 39.44919967651367, 1.582450032234192, 31.229000091552734, 38.21760177612305, 1.4331599473953247, 30.573999404907227, 35.5900993347168, 1.791450023651123, 30.573999404907227, 36.59049987792969, 1.679479956626892, 31.137699127197266, 37.18870162963867, 1.4331599473953247, 31.332599639892578, 37.206600189208984, 1.1868300437927246, 31.1481990814209, 36.46590042114258, 1.074869990348816, 30.573999404907227, 34.39580154418945, 0, 30.573999404907227, 35.3114013671875, 0, 31.111499786376953, 35.98820114135742, 0, 31.290599822998047, 36.187198638916016, 0, 31.111499786376953, 35.669700622558594, 0, 30.573999404907227, 0, 0, 40.12839889526367, 0, 0, 40.12839889526367, 4.004499912261963, -1.7077000141143799, 39.501399993896484, 4.339280128479004, 0, 39.501399993896484, 3.8207099437713623, -1.6290700435638428, 37.97869873046875, 4.140230178833008, 0, 37.97869873046875, 2.314160108566284, -0.985912024974823, 36.09769821166992, 2.5080299377441406, 0, 36.09769821166992, 2.3503799438476562, -1.0000300407409668, 34.39580154418945, 2.547840118408203, 0, 34.39580154418945, 0, 0, 40.12839889526367, 3.0849199295043945, -3.0849199295043945, 39.501399993896484, 2.943150043487549, -2.943150043487549, 37.97869873046875, 1.782039999961853, -1.782039999961853, 36.09769821166992, 1.8089599609375, -1.8089599609375, 34.39580154418945, 0, 0, 40.12839889526367, 1.7077000141143799, -4.004499912261963, 39.501399993896484, 1.6290700435638428, -3.8207099437713623, 37.97869873046875, 0.985912024974823, -2.314160108566284, 36.09769821166992, 1.0000300407409668, -2.3503799438476562, 34.39580154418945, 0, 0, 40.12839889526367, 0, -4.339280128479004, 39.501399993896484, 0, -4.140230178833008, 37.97869873046875, 0, -2.5080299377441406, 36.09769821166992, 0, -2.547840118408203, 34.39580154418945, 0, 0, 40.12839889526367, 0, 0, 40.12839889526367, -1.7077000141143799, -4.004499912261963, 39.501399993896484, 0, -4.339280128479004, 39.501399993896484, -1.6290700435638428, -3.8207099437713623, 37.97869873046875, 0, -4.140230178833008, 37.97869873046875, -0.985912024974823, -2.314160108566284, 36.09769821166992, 0, -2.5080299377441406, 36.09769821166992, -1.0000300407409668, -2.3503799438476562, 34.39580154418945, 0, -2.547840118408203, 34.39580154418945, 0, 0, 40.12839889526367, -3.0849199295043945, -3.0849199295043945, 39.501399993896484, -2.943150043487549, -2.943150043487549, 37.97869873046875, -1.782039999961853, -1.782039999961853, 36.09769821166992, -1.8089599609375, -1.8089599609375, 34.39580154418945, 0, 0, 40.12839889526367, -4.004499912261963, -1.7077000141143799, 39.501399993896484, -3.8207099437713623, -1.6290700435638428, 37.97869873046875, -2.314160108566284, -0.985912024974823, 36.09769821166992, -2.3503799438476562, -1.0000300407409668, 34.39580154418945, 0, 0, 40.12839889526367, -4.339280128479004, 0, 39.501399993896484, -4.140230178833008, 0, 37.97869873046875, -2.5080299377441406, 0, 36.09769821166992, -2.547840118408203, 0, 34.39580154418945, 0, 0, 40.12839889526367, 0, 0, 40.12839889526367, -4.004499912261963, 1.7077000141143799, 39.501399993896484, -4.339280128479004, 0, 39.501399993896484, -3.8207099437713623, 1.6290700435638428, 37.97869873046875, -4.140230178833008, 0, 37.97869873046875, -2.314160108566284, 0.985912024974823, 36.09769821166992, -2.5080299377441406, 0, 36.09769821166992, -2.3503799438476562, 1.0000300407409668, 34.39580154418945, -2.547840118408203, 0, 34.39580154418945, 0, 0, 40.12839889526367, -3.0849199295043945, 3.0849199295043945, 39.501399993896484, -2.943150043487549, 2.943150043487549, 37.97869873046875, -1.782039999961853, 1.782039999961853, 36.09769821166992, -1.8089599609375, 1.8089599609375, 34.39580154418945, 0, 0, 40.12839889526367, -1.7077000141143799, 4.004499912261963, 39.501399993896484, -1.6290700435638428, 3.8207099437713623, 37.97869873046875, -0.985912024974823, 2.314160108566284, 36.09769821166992, -1.0000300407409668, 2.3503799438476562, 34.39580154418945, 0, 0, 40.12839889526367, 0, 4.339280128479004, 39.501399993896484, 0, 4.140230178833008, 37.97869873046875, 0, 2.5080299377441406, 36.09769821166992, 0, 2.547840118408203, 34.39580154418945, 0, 0, 40.12839889526367, 0, 0, 40.12839889526367, 1.7077000141143799, 4.004499912261963, 39.501399993896484, 0, 4.339280128479004, 39.501399993896484, 1.6290700435638428, 3.8207099437713623, 37.97869873046875, 0, 4.140230178833008, 37.97869873046875, 0.985912024974823, 2.314160108566284, 36.09769821166992, 0, 2.5080299377441406, 36.09769821166992, 1.0000300407409668, 2.3503799438476562, 34.39580154418945, 0, 2.547840118408203, 34.39580154418945, 0, 0, 40.12839889526367, 3.0849199295043945, 3.0849199295043945, 39.501399993896484, 2.943150043487549, 2.943150043487549, 37.97869873046875, 1.782039999961853, 1.782039999961853, 36.09769821166992, 1.8089599609375, 1.8089599609375, 34.39580154418945, 0, 0, 40.12839889526367, 4.004499912261963, 1.7077000141143799, 39.501399993896484, 3.8207099437713623, 1.6290700435638428, 37.97869873046875, 2.314160108566284, 0.985912024974823, 36.09769821166992, 2.3503799438476562, 1.0000300407409668, 34.39580154418945, 0, 0, 40.12839889526367, 4.339280128479004, 0, 39.501399993896484, 4.140230178833008, 0, 37.97869873046875, 2.5080299377441406, 0, 36.09769821166992, 2.547840118408203, 0, 34.39580154418945, 2.547840118408203, 0, 34.39580154418945, 2.3503799438476562, -1.0000300407409668, 34.39580154418945, 5.361800193786621, -2.2813100814819336, 33.261199951171875, 5.812250137329102, 0, 33.261199951171875, 9.695320129394531, -4.125110149383545, 32.484901428222656, 10.50979995727539, 0, 32.484901428222656, 13.58810043334961, -5.781400203704834, 31.708599090576172, 14.729700088500977, 0, 31.708599090576172, 15.27750015258789, -6.5001702308654785, 30.573999404907227, 16.56089973449707, 0, 30.573999404907227, 1.8089599609375, -1.8089599609375, 34.39580154418945, 4.126699924468994, -4.126699924468994, 33.261199951171875, 7.461979866027832, -7.461979866027832, 32.484901428222656, 10.458100318908691, -10.458100318908691, 31.708599090576172, 11.758299827575684, -11.758299827575684, 30.573999404907227, 1.0000300407409668, -2.3503799438476562, 34.39580154418945, 2.2813100814819336, -5.361800193786621, 33.261199951171875, 4.125110149383545, -9.695320129394531, 32.484901428222656, 5.781400203704834, -13.58810043334961, 31.708599090576172, 6.5001702308654785, -15.27750015258789, 30.573999404907227, 0, -2.547840118408203, 34.39580154418945, 0, -5.812250137329102, 33.261199951171875, 0, -10.50979995727539, 32.484901428222656, 0, -14.729700088500977, 31.708599090576172, 0, -16.56089973449707, 30.573999404907227, 0, -2.547840118408203, 34.39580154418945, -1.0000300407409668, -2.3503799438476562, 34.39580154418945, -2.2813100814819336, -5.361800193786621, 33.261199951171875, 0, -5.812250137329102, 33.261199951171875, -4.125110149383545, -9.695320129394531, 32.484901428222656, 0, -10.50979995727539, 32.484901428222656, -5.781400203704834, -13.58810043334961, 31.708599090576172, 0, -14.729700088500977, 31.708599090576172, -6.5001702308654785, -15.27750015258789, 30.573999404907227, 0, -16.56089973449707, 30.573999404907227, -1.8089599609375, -1.8089599609375, 34.39580154418945, -4.126699924468994, -4.126699924468994, 33.261199951171875, -7.461979866027832, -7.461979866027832, 32.484901428222656, -10.458100318908691, -10.458100318908691, 31.708599090576172, -11.758299827575684, -11.758299827575684, 30.573999404907227, -2.3503799438476562, -1.0000300407409668, 34.39580154418945, -5.361800193786621, -2.2813100814819336, 33.261199951171875, -9.695320129394531, -4.125110149383545, 32.484901428222656, -13.58810043334961, -5.781400203704834, 31.708599090576172, -15.27750015258789, -6.5001702308654785, 30.573999404907227, -2.547840118408203, 0, 34.39580154418945, -5.812250137329102, 0, 33.261199951171875, -10.50979995727539, 0, 32.484901428222656, -14.729700088500977, 0, 31.708599090576172, -16.56089973449707, 0, 30.573999404907227, -2.547840118408203, 0, 34.39580154418945, -2.3503799438476562, 1.0000300407409668, 34.39580154418945, -5.361800193786621, 2.2813100814819336, 33.261199951171875, -5.812250137329102, 0, 33.261199951171875, -9.695320129394531, 4.125110149383545, 32.484901428222656, -10.50979995727539, 0, 32.484901428222656, -13.58810043334961, 5.781400203704834, 31.708599090576172, -14.729700088500977, 0, 31.708599090576172, -15.27750015258789, 6.5001702308654785, 30.573999404907227, -16.56089973449707, 0, 30.573999404907227, -1.8089599609375, 1.8089599609375, 34.39580154418945, -4.126699924468994, 4.126699924468994, 33.261199951171875, -7.461979866027832, 7.461979866027832, 32.484901428222656, -10.458100318908691, 10.458100318908691, 31.708599090576172, -11.758299827575684, 11.758299827575684, 30.573999404907227, -1.0000300407409668, 2.3503799438476562, 34.39580154418945, -2.2813100814819336, 5.361800193786621, 33.261199951171875, -4.125110149383545, 9.695320129394531, 32.484901428222656, -5.781400203704834, 13.58810043334961, 31.708599090576172, -6.5001702308654785, 15.27750015258789, 30.573999404907227, 0, 2.547840118408203, 34.39580154418945, 0, 5.812250137329102, 33.261199951171875, 0, 10.50979995727539, 32.484901428222656, 0, 14.729700088500977, 31.708599090576172, 0, 16.56089973449707, 30.573999404907227, 0, 2.547840118408203, 34.39580154418945, 1.0000300407409668, 2.3503799438476562, 34.39580154418945, 2.2813100814819336, 5.361800193786621, 33.261199951171875, 0, 5.812250137329102, 33.261199951171875, 4.125110149383545, 9.695320129394531, 32.484901428222656, 0, 10.50979995727539, 32.484901428222656, 5.781400203704834, 13.58810043334961, 31.708599090576172, 0, 14.729700088500977, 31.708599090576172, 6.5001702308654785, 15.27750015258789, 30.573999404907227, 0, 16.56089973449707, 30.573999404907227, 1.8089599609375, 1.8089599609375, 34.39580154418945, 4.126699924468994, 4.126699924468994, 33.261199951171875, 7.461979866027832, 7.461979866027832, 32.484901428222656, 10.458100318908691, 10.458100318908691, 31.708599090576172, 11.758299827575684, 11.758299827575684, 30.573999404907227, 2.3503799438476562, 1.0000300407409668, 34.39580154418945, 5.361800193786621, 2.2813100814819336, 33.261199951171875, 9.695320129394531, 4.125110149383545, 32.484901428222656, 13.58810043334961, 5.781400203704834, 31.708599090576172, 15.27750015258789, 6.5001702308654785, 30.573999404907227, 2.547840118408203, 0, 34.39580154418945, 5.812250137329102, 0, 33.261199951171875, 10.50979995727539, 0, 32.484901428222656, 14.729700088500977, 0, 31.708599090576172, 16.56089973449707, 0, 30.573999404907227
  };
  // scale and translate vertices to unit size and origin center
  GLfloat factor = size / 50.f;
  int nVertices = sizeof(vertices) / sizeof(GLfloat);
  for (int i = 0; i < nVertices; ++i) {
    vertices[i] *= factor;
  }
  for (int i = 2; i < nVertices; i += 3) {
    vertices[i] -= 0.4f;
  }

  // define texture coordinates (3D)
  GLfloat texCoords[] = {
      2, 2, 0, 1.75, 2, 0, 1.75, 1.975000023841858, 0, 2, 1.975000023841858, 0, 1.75, 1.9500000476837158, 0, 2, 1.9500000476837158, 0, 1.75, 1.9249999523162842, 0, 2, 1.9249999523162842, 0, 1.75, 1.899999976158142, 0, 2, 1.899999976158142, 0, 1.5, 2, 0, 1.5, 1.975000023841858, 0, 1.5, 1.9500000476837158, 0, 1.5, 1.9249999523162842, 0, 1.5, 1.899999976158142, 0, 1.25, 2, 0, 1.25, 1.975000023841858, 0, 1.25, 1.9500000476837158, 0, 1.25, 1.9249999523162842, 0, 1.25, 1.899999976158142, 0, 1, 2, 0, 1, 1.975000023841858, 0, 1, 1.9500000476837158, 0, 1, 1.9249999523162842, 0, 1, 1.899999976158142, 0, 1, 2, 0, 0.75, 2, 0, 0.75, 1.975000023841858, 0, 1, 1.975000023841858, 0, 0.75, 1.9500000476837158, 0, 1, 1.9500000476837158, 0, 0.75, 1.9249999523162842, 0, 1, 1.9249999523162842, 0, 0.75, 1.899999976158142, 0, 1, 1.899999976158142, 0, 0.5, 2, 0, 0.5, 1.975000023841858, 0, 0.5, 1.9500000476837158, 0, 0.5, 1.9249999523162842, 0, 0.5, 1.899999976158142, 0, 0.25, 2, 0, 0.25, 1.975000023841858, 0, 0.25, 1.9500000476837158, 0, 0.25, 1.9249999523162842, 0, 0.25, 1.899999976158142, 0, 0, 2, 0, 0, 1.975000023841858, 0, 0, 1.9500000476837158, 0, 0, 1.9249999523162842, 0, 0, 1.899999976158142, 0, 2, 2, 0, 1.75, 2, 0, 1.75, 1.975000023841858, 0, 2, 1.975000023841858, 0, 1.75, 1.9500000476837158, 0, 2, 1.9500000476837158, 0, 1.75, 1.9249999523162842, 0, 2, 1.9249999523162842, 0, 1.75, 1.899999976158142, 0, 2, 1.899999976158142, 0, 1.5, 2, 0, 1.5, 1.975000023841858, 0, 1.5, 1.9500000476837158, 0, 1.5, 1.9249999523162842, 0, 1.5, 1.899999976158142, 0, 1.25, 2, 0, 1.25, 1.975000023841858, 0, 1.25, 1.9500000476837158, 0, 1.25, 1.9249999523162842, 0, 1.25, 1.899999976158142, 0, 1, 2, 0, 1, 1.975000023841858, 0, 1, 1.9500000476837158, 0, 1, 1.9249999523162842, 0, 1, 1.899999976158142, 0, 1, 2, 0, 0.75, 2, 0, 0.75, 1.975000023841858, 0, 1, 1.975000023841858, 0, 0.75, 1.9500000476837158, 0, 1, 1.9500000476837158, 0, 0.75, 1.9249999523162842, 0, 1, 1.9249999523162842, 0, 0.75, 1.899999976158142, 0, 1, 1.899999976158142, 0, 0.5, 2, 0, 0.5, 1.975000023841858, 0, 0.5, 1.9500000476837158, 0, 0.5, 1.9249999523162842, 0, 0.5, 1.899999976158142, 0, 0.25, 2, 0, 0.25, 1.975000023841858, 0, 0.25, 1.9500000476837158, 0, 0.25, 1.9249999523162842, 0, 0.25, 1.899999976158142, 0, 0, 2, 0, 0, 1.975000023841858, 0, 0, 1.9500000476837158, 0, 0, 1.9249999523162842, 0, 0, 1.899999976158142, 0, 2, 1.899999976158142, 0, 1.75, 1.899999976158142, 0, 1.75, 1.6749999523162842, 0, 2, 1.6749999523162842, 0, 1.75, 1.4500000476837158, 0, 2, 1.4500000476837158, 0, 1.75, 1.225000023841858, 0, 2, 1.225000023841858, 0, 1.75, 1, 0, 2, 1, 0, 1.5, 1.899999976158142, 0, 1.5, 1.6749999523162842, 0, 1.5, 1.4500000476837158, 0, 1.5, 1.225000023841858, 0, 1.5, 1, 0, 1.25, 1.899999976158142, 0, 1.25, 1.6749999523162842, 0, 1.25, 1.4500000476837158, 0, 1.25, 1.225000023841858, 0, 1.25, 1, 0, 1, 1.899999976158142, 0, 1, 1.6749999523162842, 0, 1, 1.4500000476837158, 0, 1, 1.225000023841858, 0, 1, 1, 0, 1, 1.899999976158142, 0, 0.75, 1.899999976158142, 0, 0.75, 1.6749999523162842, 0, 1, 1.6749999523162842, 0, 0.75, 1.4500000476837158, 0, 1, 1.4500000476837158, 0, 0.75, 1.225000023841858, 0, 1, 1.225000023841858, 0, 0.75, 1, 0, 1, 1, 0, 0.5, 1.899999976158142, 0, 0.5, 1.6749999523162842, 0, 0.5, 1.4500000476837158, 0, 0.5, 1.225000023841858, 0, 0.5, 1, 0, 0.25, 1.899999976158142, 0, 0.25, 1.6749999523162842, 0, 0.25, 1.4500000476837158, 0, 0.25, 1.225000023841858, 0, 0.25, 1, 0, 0, 1.899999976158142, 0, 0, 1.6749999523162842, 0, 0, 1.4500000476837158, 0, 0, 1.225000023841858, 0, 0, 1, 0, 2, 1.899999976158142, 0, 1.75, 1.899999976158142, 0, 1.75, 1.6749999523162842, 0, 2, 1.6749999523162842, 0, 1.75, 1.4500000476837158, 0, 2, 1.4500000476837158, 0, 1.75, 1.225000023841858, 0, 2, 1.225000023841858, 0, 1.75, 1, 0, 2, 1, 0, 1.5, 1.899999976158142, 0, 1.5, 1.6749999523162842, 0, 1.5, 1.4500000476837158, 0, 1.5, 1.225000023841858, 0, 1.5, 1, 0, 1.25, 1.899999976158142, 0, 1.25, 1.6749999523162842, 0, 1.25, 1.4500000476837158, 0, 1.25, 1.225000023841858, 0, 1.25, 1, 0, 1, 1.899999976158142, 0, 1, 1.6749999523162842, 0, 1, 1.4500000476837158, 0, 1, 1.225000023841858, 0, 1, 1, 0, 1, 1.899999976158142, 0, 0.75, 1.899999976158142, 0, 0.75, 1.6749999523162842, 0, 1, 1.6749999523162842, 0, 0.75, 1.4500000476837158, 0, 1, 1.4500000476837158, 0, 0.75, 1.225000023841858, 0, 1, 1.225000023841858, 0, 0.75, 1, 0, 1, 1, 0, 0.5, 1.899999976158142, 0, 0.5, 1.6749999523162842, 0, 0.5, 1.4500000476837158, 0, 0.5, 1.225000023841858, 0, 0.5, 1, 0, 0.25, 1.899999976158142, 0, 0.25, 1.6749999523162842, 0, 0.25, 1.4500000476837158, 0, 0.25, 1.225000023841858, 0, 0.25, 1, 0, 0, 1.899999976158142, 0, 0, 1.6749999523162842, 0, 0, 1.4500000476837158, 0, 0, 1.225000023841858, 0, 0, 1, 0, 2, 1, 0, 1.75, 1, 0, 1.75, 0.8500000238418579, 0, 2, 0.8500000238418579, 0, 1.75, 0.699999988079071, 0, 2, 0.699999988079071, 0, 1.75, 0.550000011920929, 0, 2, 0.550000011920929, 0, 1.75, 0.4000000059604645, 0, 2, 0.4000000059604645, 0, 1.5, 1, 0, 1.5, 0.8500000238418579, 0, 1.5, 0.699999988079071, 0, 1.5, 0.550000011920929, 0, 1.5, 0.4000000059604645, 0, 1.25, 1, 0, 1.25, 0.8500000238418579, 0, 1.25, 0.699999988079071, 0, 1.25, 0.550000011920929, 0, 1.25, 0.4000000059604645, 0, 1, 1, 0, 1, 0.8500000238418579, 0, 1, 0.699999988079071, 0, 1, 0.550000011920929, 0, 1, 0.4000000059604645, 0, 1, 1, 0, 0.75, 1, 0, 0.75, 0.8500000238418579, 0, 1, 0.8500000238418579, 0, 0.75, 0.699999988079071, 0, 1, 0.699999988079071, 0, 0.75, 0.550000011920929, 0, 1, 0.550000011920929, 0, 0.75, 0.4000000059604645, 0, 1, 0.4000000059604645, 0, 0.5, 1, 0, 0.5, 0.8500000238418579, 0, 0.5, 0.699999988079071, 0, 0.5, 0.550000011920929, 0, 0.5, 0.4000000059604645, 0, 0.25, 1, 0, 0.25, 0.8500000238418579, 0, 0.25, 0.699999988079071, 0, 0.25, 0.550000011920929, 0, 0.25, 0.4000000059604645, 0, 0, 1, 0, 0, 0.8500000238418579, 0, 0, 0.699999988079071, 0, 0, 0.550000011920929, 0, 0, 0.4000000059604645, 0, 2, 1, 0, 1.75, 1, 0, 1.75, 0.8500000238418579, 0, 2, 0.8500000238418579, 0, 1.75, 0.699999988079071, 0, 2, 0.699999988079071, 0, 1.75, 0.550000011920929, 0, 2, 0.550000011920929, 0, 1.75, 0.4000000059604645, 0, 2, 0.4000000059604645, 0, 1.5, 1, 0, 1.5, 0.8500000238418579, 0, 1.5, 0.699999988079071, 0, 1.5, 0.550000011920929, 0, 1.5, 0.4000000059604645, 0, 1.25, 1, 0, 1.25, 0.8500000238418579, 0, 1.25, 0.699999988079071, 0, 1.25, 0.550000011920929, 0, 1.25, 0.4000000059604645, 0, 1, 1, 0, 1, 0.8500000238418579, 0, 1, 0.699999988079071, 0, 1, 0.550000011920929, 0, 1, 0.4000000059604645, 0, 1, 1, 0, 0.75, 1, 0, 0.75, 0.8500000238418579, 0, 1, 0.8500000238418579, 0, 0.75, 0.699999988079071, 0, 1, 0.699999988079071, 0, 0.75, 0.550000011920929, 0, 1, 0.550000011920929, 0, 0.75, 0.4000000059604645, 0, 1, 0.4000000059604645, 0, 0.5, 1, 0, 0.5, 0.8500000238418579, 0, 0.5, 0.699999988079071, 0, 0.5, 0.550000011920929, 0, 0.5, 0.4000000059604645, 0, 0.25, 1, 0, 0.25, 0.8500000238418579, 0, 0.25, 0.699999988079071, 0, 0.25, 0.550000011920929, 0, 0.25, 0.4000000059604645, 0, 0, 1, 0, 0, 0.8500000238418579, 0, 0, 0.699999988079071, 0, 0, 0.550000011920929, 0, 0, 0.4000000059604645, 0, 2, 0.4000000059604645, 0, 1.75, 0.4000000059604645, 0, 1.75, 0.30000001192092896, 0, 2, 0.30000001192092896, 0, 1.75, 0.20000000298023224, 0, 2, 0.20000000298023224, 0, 1.75, 0.10000000149011612, 0, 2, 0.10000000149011612, 0, 1.75, 0, 0, 2, 0, 0, 1.5, 0.4000000059604645, 0, 1.5, 0.30000001192092896, 0, 1.5, 0.20000000298023224, 0, 1.5, 0.10000000149011612, 0, 1.5, 0, 0, 1.25, 0.4000000059604645, 0, 1.25, 0.30000001192092896, 0, 1.25, 0.20000000298023224, 0, 1.25, 0.10000000149011612, 0, 1.25, 0, 0, 1, 0.4000000059604645, 0, 1, 0.30000001192092896, 0, 1, 0.20000000298023224, 0, 1, 0.10000000149011612, 0, 1, 0, 0, 1, 0.4000000059604645, 0, 0.75, 0.4000000059604645, 0, 0.75, 0.30000001192092896, 0, 1, 0.30000001192092896, 0, 0.75, 0.20000000298023224, 0, 1, 0.20000000298023224, 0, 0.75, 0.10000000149011612, 0, 1, 0.10000000149011612, 0, 0.75, 0, 0, 1, 0, 0, 0.5, 0.4000000059604645, 0, 0.5, 0.30000001192092896, 0, 0.5, 0.20000000298023224, 0, 0.5, 0.10000000149011612, 0, 0.5, 0, 0, 0.25, 0.4000000059604645, 0, 0.25, 0.30000001192092896, 0, 0.25, 0.20000000298023224, 0, 0.25, 0.10000000149011612, 0, 0.25, 0, 0, 0, 0.4000000059604645, 0, 0, 0.30000001192092896, 0, 0, 0.20000000298023224, 0, 0, 0.10000000149011612, 0, 0, 0, 0, 2, 0.4000000059604645, 0, 1.75, 0.4000000059604645, 0, 1.75, 0.30000001192092896, 0, 2, 0.30000001192092896, 0, 1.75, 0.20000000298023224, 0, 2, 0.20000000298023224, 0, 1.75, 0.10000000149011612, 0, 2, 0.10000000149011612, 0, 1.75, 0, 0, 2, 0, 0, 1.5, 0.4000000059604645, 0, 1.5, 0.30000001192092896, 0, 1.5, 0.20000000298023224, 0, 1.5, 0.10000000149011612, 0, 1.5, 0, 0, 1.25, 0.4000000059604645, 0, 1.25, 0.30000001192092896, 0, 1.25, 0.20000000298023224, 0, 1.25, 0.10000000149011612, 0, 1.25, 0, 0, 1, 0.4000000059604645, 0, 1, 0.30000001192092896, 0, 1, 0.20000000298023224, 0, 1, 0.10000000149011612, 0, 1, 0, 0, 1, 0.4000000059604645, 0, 0.75, 0.4000000059604645, 0, 0.75, 0.30000001192092896, 0, 1, 0.30000001192092896, 0, 0.75, 0.20000000298023224, 0, 1, 0.20000000298023224, 0, 0.75, 0.10000000149011612, 0, 1, 0.10000000149011612, 0, 0.75, 0, 0, 1, 0, 0, 0.5, 0.4000000059604645, 0, 0.5, 0.30000001192092896, 0, 0.5, 0.20000000298023224, 0, 0.5, 0.10000000149011612, 0, 0.5, 0, 0, 0.25, 0.4000000059604645, 0, 0.25, 0.30000001192092896, 0, 0.25, 0.20000000298023224, 0, 0.25, 0.10000000149011612, 0, 0.25, 0, 0, 0, 0.4000000059604645, 0, 0, 0.30000001192092896, 0, 0, 0.20000000298023224, 0, 0, 0.10000000149011612, 0, 0, 0, 0, 1, 1, 0, 0.875, 1, 0, 0.875, 0.875, 0, 1, 0.875, 0, 0.875, 0.75, 0, 1, 0.75, 0, 0.875, 0.625, 0, 1, 0.625, 0, 0.875, 0.5, 0, 1, 0.5, 0, 0.75, 1, 0, 0.75, 0.875, 0, 0.75, 0.75, 0, 0.75, 0.625, 0, 0.75, 0.5, 0, 0.625, 1, 0, 0.625, 0.875, 0, 0.625, 0.75, 0, 0.625, 0.625, 0, 0.625, 0.5, 0, 0.5, 1, 0, 0.5, 0.875, 0, 0.5, 0.75, 0, 0.5, 0.625, 0, 0.5, 0.5, 0, 0.5, 1, 0, 0.375, 1, 0, 0.375, 0.875, 0, 0.5, 0.875, 0, 0.375, 0.75, 0, 0.5, 0.75, 0, 0.375, 0.625, 0, 0.5, 0.625, 0, 0.375, 0.5, 0, 0.5, 0.5, 0, 0.25, 1, 0, 0.25, 0.875, 0, 0.25, 0.75, 0, 0.25, 0.625, 0, 0.25, 0.5, 0, 0.125, 1, 0, 0.125, 0.875, 0, 0.125, 0.75, 0, 0.125, 0.625, 0, 0.125, 0.5, 0, 0, 1, 0, 0, 0.875, 0, 0, 0.75, 0, 0, 0.625, 0, 0, 0.5, 0, 1, 0.5, 0, 0.875, 0.5, 0, 0.875, 0.375, 0, 1, 0.375, 0, 0.875, 0.25, 0, 1, 0.25, 0, 0.875, 0.125, 0, 1, 0.125, 0, 0.875, 0, 0, 1, 0, 0, 0.75, 0.5, 0, 0.75, 0.375, 0, 0.75, 0.25, 0, 0.75, 0.125, 0, 0.75, 0, 0, 0.625, 0.5, 0, 0.625, 0.375, 0, 0.625, 0.25, 0, 0.625, 0.125, 0, 0.625, 0, 0, 0.5, 0.5, 0, 0.5, 0.375, 0, 0.5, 0.25, 0, 0.5, 0.125, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0.375, 0.5, 0, 0.375, 0.375, 0, 0.5, 0.375, 0, 0.375, 0.25, 0, 0.5, 0.25, 0, 0.375, 0.125, 0, 0.5, 0.125, 0, 0.375, 0, 0, 0.5, 0, 0, 0.25, 0.5, 0, 0.25, 0.375, 0, 0.25, 0.25, 0, 0.25, 0.125, 0, 0.25, 0, 0, 0.125, 0.5, 0, 0.125, 0.375, 0, 0.125, 0.25, 0, 0.125, 0.125, 0, 0.125, 0, 0, 0, 0.5, 0, 0, 0.375, 0, 0, 0.25, 0, 0, 0.125, 0, 0, 0, 0, 0.5, 0, 0, 0.625, 0, 0, 0.625, 0.22499999403953552, 0, 0.5, 0.22499999403953552, 0, 0.625, 0.44999998807907104, 0, 0.5, 0.44999998807907104, 0, 0.625, 0.675000011920929, 0, 0.5, 0.675000011920929, 0, 0.625, 0.8999999761581421, 0, 0.5, 0.8999999761581421, 0, 0.75, 0, 0, 0.75, 0.22499999403953552, 0, 0.75, 0.44999998807907104, 0, 0.75, 0.675000011920929, 0, 0.75, 0.8999999761581421, 0, 0.875, 0, 0, 0.875, 0.22499999403953552, 0, 0.875, 0.44999998807907104, 0, 0.875, 0.675000011920929, 0, 0.875, 0.8999999761581421, 0, 1, 0, 0, 1, 0.22499999403953552, 0, 1, 0.44999998807907104, 0, 1, 0.675000011920929, 0, 1, 0.8999999761581421, 0, 0, 0, 0, 0.125, 0, 0, 0.125, 0.22499999403953552, 0, 0, 0.22499999403953552, 0, 0.125, 0.44999998807907104, 0, 0, 0.44999998807907104, 0, 0.125, 0.675000011920929, 0, 0, 0.675000011920929, 0, 0.125, 0.8999999761581421, 0, 0, 0.8999999761581421, 0, 0.25, 0, 0, 0.25, 0.22499999403953552, 0, 0.25, 0.44999998807907104, 0, 0.25, 0.675000011920929, 0, 0.25, 0.8999999761581421, 0, 0.375, 0, 0, 0.375, 0.22499999403953552, 0, 0.375, 0.44999998807907104, 0, 0.375, 0.675000011920929, 0, 0.375, 0.8999999761581421, 0, 0.5, 0, 0, 0.5, 0.22499999403953552, 0, 0.5, 0.44999998807907104, 0, 0.5, 0.675000011920929, 0, 0.5, 0.8999999761581421, 0, 0.5, 0.8999999761581421, 0, 0.625, 0.8999999761581421, 0, 0.625, 0.925000011920929, 0, 0.5, 0.925000011920929, 0, 0.625, 0.949999988079071, 0, 0.5, 0.949999988079071, 0, 0.625, 0.9750000238418579, 0, 0.5, 0.9750000238418579, 0, 0.625, 1, 0, 0.5, 1, 0, 0.75, 0.8999999761581421, 0, 0.75, 0.925000011920929, 0, 0.75, 0.949999988079071, 0, 0.75, 0.9750000238418579, 0, 0.75, 1, 0, 0.875, 0.8999999761581421, 0, 0.875, 0.925000011920929, 0, 0.875, 0.949999988079071, 0, 0.875, 0.9750000238418579, 0, 0.875, 1, 0, 1, 0.8999999761581421, 0, 1, 0.925000011920929, 0, 1, 0.949999988079071, 0, 1, 0.9750000238418579, 0, 1, 1, 0, 0, 0.8999999761581421, 0, 0.125, 0.8999999761581421, 0, 0.125, 0.925000011920929, 0, 0, 0.925000011920929, 0, 0.125, 0.949999988079071, 0, 0, 0.949999988079071, 0, 0.125, 0.9750000238418579, 0, 0, 0.9750000238418579, 0, 0.125, 1, 0, 0, 1, 0, 0.25, 0.8999999761581421, 0, 0.25, 0.925000011920929, 0, 0.25, 0.949999988079071, 0, 0.25, 0.9750000238418579, 0, 0.25, 1, 0, 0.375, 0.8999999761581421, 0, 0.375, 0.925000011920929, 0, 0.375, 0.949999988079071, 0, 0.375, 0.9750000238418579, 0, 0.375, 1, 0, 0.5, 0.8999999761581421, 0, 0.5, 0.925000011920929, 0, 0.5, 0.949999988079071, 0, 0.5, 0.9750000238418579, 0, 0.5, 1, 0, 1, 1, 0, 0.875, 1, 0, 0.875, 0.75, 0, 1, 0.75, 0, 0.875, 0.5, 0, 1, 0.5, 0, 0.875, 0.25, 0, 1, 0.25, 0, 0.875, 0, 0, 1, 0, 0, 0.75, 1, 0, 0.75, 0.75, 0, 0.75, 0.5, 0, 0.75, 0.25, 0, 0.75, 0, 0, 0.625, 1, 0, 0.625, 0.75, 0, 0.625, 0.5, 0, 0.625, 0.25, 0, 0.625, 0, 0, 0.5, 1, 0, 0.5, 0.75, 0, 0.5, 0.5, 0, 0.5, 0.25, 0, 0.5, 0, 0, 0.5, 1, 0, 0.375, 1, 0, 0.375, 0.75, 0, 0.5, 0.75, 0, 0.375, 0.5, 0, 0.5, 0.5, 0, 0.375, 0.25, 0, 0.5, 0.25, 0, 0.375, 0, 0, 0.5, 0, 0, 0.25, 1, 0, 0.25, 0.75, 0, 0.25, 0.5, 0, 0.25, 0.25, 0, 0.25, 0, 0, 0.125, 1, 0, 0.125, 0.75, 0, 0.125, 0.5, 0, 0.125, 0.25, 0, 0.125, 0, 0, 0, 1, 0, 0, 0.75, 0, 0, 0.5, 0, 0, 0.25, 0, 0, 0, 0, 1, 1, 0, 0.875, 1, 0, 0.875, 0.75, 0, 1, 0.75, 0, 0.875, 0.5, 0, 1, 0.5, 0, 0.875, 0.25, 0, 1, 0.25, 0, 0.875, 0, 0, 1, 0, 0, 0.75, 1, 0, 0.75, 0.75, 0, 0.75, 0.5, 0, 0.75, 0.25, 0, 0.75, 0, 0, 0.625, 1, 0, 0.625, 0.75, 0, 0.625, 0.5, 0, 0.625, 0.25, 0, 0.625, 0, 0, 0.5, 1, 0, 0.5, 0.75, 0, 0.5, 0.5, 0, 0.5, 0.25, 0, 0.5, 0, 0, 0.5, 1, 0, 0.375, 1, 0, 0.375, 0.75, 0, 0.5, 0.75, 0, 0.375, 0.5, 0, 0.5, 0.5, 0, 0.375, 0.25, 0, 0.5, 0.25, 0, 0.375, 0, 0, 0.5, 0, 0, 0.25, 1, 0, 0.25, 0.75, 0, 0.25, 0.5, 0, 0.25, 0.25, 0, 0.25, 0, 0, 0.125, 1, 0, 0.125, 0.75, 0, 0.125, 0.5, 0, 0.125, 0.25, 0, 0.125, 0, 0, 0, 1, 0, 0, 0.75, 0, 0, 0.5, 0, 0, 0.25, 0, 0, 0, 0, 1, 1, 0, 0.875, 1, 0, 0.875, 0.75, 0, 1, 0.75, 0, 0.875, 0.5, 0, 1, 0.5, 0, 0.875, 0.25, 0, 1, 0.25, 0, 0.875, 0, 0, 1, 0, 0, 0.75, 1, 0, 0.75, 0.75, 0, 0.75, 0.5, 0, 0.75, 0.25, 0, 0.75, 0, 0, 0.625, 1, 0, 0.625, 0.75, 0, 0.625, 0.5, 0, 0.625, 0.25, 0, 0.625, 0, 0, 0.5, 1, 0, 0.5, 0.75, 0, 0.5, 0.5, 0, 0.5, 0.25, 0, 0.5, 0, 0, 0.5, 1, 0, 0.375, 1, 0, 0.375, 0.75, 0, 0.5, 0.75, 0, 0.375, 0.5, 0, 0.5, 0.5, 0, 0.375, 0.25, 0, 0.5, 0.25, 0, 0.375, 0, 0, 0.5, 0, 0, 0.25, 1, 0, 0.25, 0.75, 0, 0.25, 0.5, 0, 0.25, 0.25, 0, 0.25, 0, 0, 0.125, 1, 0, 0.125, 0.75, 0, 0.125, 0.5, 0, 0.125, 0.25, 0, 0.125, 0, 0, 0, 1, 0, 0, 0.75, 0, 0, 0.5, 0, 0, 0.25, 0, 0, 0, 0, 1, 1, 0, 0.875, 1, 0, 0.875, 0.75, 0, 1, 0.75, 0, 0.875, 0.5, 0, 1, 0.5, 0, 0.875, 0.25, 0, 1, 0.25, 0, 0.875, 0, 0, 1, 0, 0, 0.75, 1, 0, 0.75, 0.75, 0, 0.75, 0.5, 0, 0.75, 0.25, 0, 0.75, 0, 0, 0.625, 1, 0, 0.625, 0.75, 0, 0.625, 0.5, 0, 0.625, 0.25, 0, 0.625, 0, 0, 0.5, 1, 0, 0.5, 0.75, 0, 0.5, 0.5, 0, 0.5, 0.25, 0, 0.5, 0, 0, 0.5, 1, 0, 0.375, 1, 0, 0.375, 0.75, 0, 0.5, 0.75, 0, 0.375, 0.5, 0, 0.5, 0.5, 0, 0.375, 0.25, 0, 0.5, 0.25, 0, 0.375, 0, 0, 0.5, 0, 0, 0.25, 1, 0, 0.25, 0.75, 0, 0.25, 0.5, 0, 0.25, 0.25, 0, 0.25, 0, 0, 0.125, 1, 0, 0.125, 0.75, 0, 0.125, 0.5, 0, 0.125, 0.25, 0, 0.125, 0, 0, 0, 1, 0, 0, 0.75, 0, 0, 0.5, 0, 0, 0.25, 0, 0, 0, 0
  };
  // mirror texture coordinates
  int nTexCoords = sizeof(texCoords) / sizeof(GLfloat);
  for (int i = 1; i < nTexCoords; i += 3) {
    texCoords[i] = 2.f - texCoords[i];
  }

  // define indices (1024 triangles)
  const GLuint indices[] = {
      0, 1, 2, 2, 3, 0, 3, 2, 4, 4, 5, 3, 5, 4, 6, 6, 7, 5, 7, 6, 8, 8, 9, 7, 1, 10, 11, 11, 2, 1, 2, 11, 12, 12, 4, 2, 4, 12, 13, 13, 6, 4, 6, 13, 14, 14, 8, 6, 10, 15, 16, 16, 11, 10, 11, 16, 17, 17, 12, 11, 12, 17, 18, 18, 13, 12, 13, 18, 19, 19, 14, 13, 15, 20, 21, 21, 16, 15, 16, 21, 22, 22, 17, 16, 17, 22, 23, 23, 18, 17, 18, 23, 24, 24, 19, 18, 25, 26, 27, 27, 28, 25, 28, 27, 29, 29, 30, 28, 30, 29, 31, 31, 32, 30, 32, 31, 33, 33, 34, 32, 26, 35, 36, 36, 27, 26, 27, 36, 37, 37, 29, 27, 29, 37, 38, 38, 31, 29, 31, 38, 39, 39, 33, 31, 35, 40, 41, 41, 36, 35, 36, 41, 42, 42, 37, 36, 37, 42, 43, 43, 38, 37, 38, 43, 44, 44, 39, 38, 40, 45, 46, 46, 41, 40, 41, 46, 47, 47, 42, 41, 42, 47, 48, 48, 43, 42, 43, 48, 49, 49, 44, 43, 50, 51, 52, 52, 53, 50, 53, 52, 54, 54, 55, 53, 55, 54, 56, 56, 57, 55, 57, 56, 58, 58, 59, 57, 51, 60, 61, 61, 52, 51, 52, 61, 62, 62, 54, 52, 54, 62, 63, 63, 56, 54, 56, 63, 64, 64, 58, 56, 60, 65, 66, 66, 61, 60, 61, 66, 67, 67, 62, 61, 62, 67, 68, 68, 63, 62, 63, 68, 69, 69, 64, 63, 65, 70, 71, 71, 66, 65, 66, 71, 72, 72, 67, 66, 67, 72, 73, 73, 68, 67, 68, 73, 74, 74, 69, 68, 75, 76, 77, 77, 78, 75, 78, 77, 79, 79, 80, 78, 80, 79, 81, 81, 82, 80, 82, 81, 83, 83, 84, 82, 76, 85, 86, 86, 77, 76, 77, 86, 87, 87, 79, 77, 79, 87, 88, 88, 81, 79, 81, 88, 89, 89, 83, 81, 85, 90, 91, 91, 86, 85, 86, 91, 92, 92, 87, 86, 87, 92, 93, 93, 88, 87, 88, 93, 94, 94, 89, 88, 90, 95, 96, 96, 91, 90, 91, 96, 97, 97, 92, 91, 92, 97, 98, 98, 93, 92, 93, 98, 99, 99, 94, 93, 100, 101, 102, 102, 103, 100, 103, 102, 104, 104, 105, 103, 105, 104, 106, 106, 107, 105, 107, 106, 108, 108, 109, 107, 101, 110, 111, 111, 102, 101, 102, 111, 112, 112, 104, 102, 104, 112, 113, 113, 106, 104, 106, 113, 114, 114, 108, 106, 110, 115, 116, 116, 111, 110, 111, 116, 117, 117, 112, 111, 112, 117, 118, 118, 113, 112, 113, 118, 119, 119, 114, 113, 115, 120, 121, 121, 116, 115, 116, 121, 122, 122, 117, 116, 117, 122, 123, 123, 118, 117, 118, 123, 124, 124, 119, 118, 125, 126, 127, 127, 128, 125, 128, 127, 129, 129, 130, 128, 130, 129, 131, 131, 132, 130, 132, 131, 133, 133, 134, 132, 126, 135, 136, 136, 127, 126, 127, 136, 137, 137, 129, 127, 129, 137, 138, 138, 131, 129, 131, 138, 139, 139, 133, 131, 135, 140, 141, 141, 136, 135, 136, 141, 142, 142, 137, 136, 137, 142, 143, 143, 138, 137, 138, 143, 144, 144, 139, 138, 140, 145, 146, 146, 141, 140, 141, 146, 147, 147, 142, 141, 142, 147, 148, 148, 143, 142, 143, 148, 149, 149, 144, 143, 150, 151, 152, 152, 153, 150, 153, 152, 154, 154, 155, 153, 155, 154, 156, 156, 157, 155, 157, 156, 158, 158, 159, 157, 151, 160, 161, 161, 152, 151, 152, 161, 162, 162, 154, 152, 154, 162, 163, 163, 156, 154, 156, 163, 164, 164, 158, 156, 160, 165, 166, 166, 161, 160, 161, 166, 167, 167, 162, 161, 162, 167, 168, 168, 163, 162, 163, 168, 169, 169, 164, 163, 165, 170, 171, 171, 166, 165, 166, 171, 172, 172, 167, 166, 167, 172, 173, 173, 168, 167, 168, 173, 174, 174, 169, 168, 175, 176, 177, 177, 178, 175, 178, 177, 179, 179, 180, 178, 180, 179, 181, 181, 182, 180, 182, 181, 183, 183, 184, 182, 176, 185, 186, 186, 177, 176, 177, 186, 187, 187, 179, 177, 179, 187, 188, 188, 181, 179, 181, 188, 189, 189, 183, 181, 185, 190, 191, 191, 186, 185, 186, 191, 192, 192, 187, 186, 187, 192, 193, 193, 188, 187, 188, 193, 194, 194, 189, 188, 190, 195, 196, 196, 191, 190, 191, 196, 197, 197, 192, 191, 192, 197, 198, 198, 193, 192, 193, 198, 199, 199, 194, 193, 200, 201, 202, 202, 203, 200, 203, 202, 204, 204, 205, 203, 205, 204, 206, 206, 207, 205, 207, 206, 208, 208, 209, 207, 201, 210, 211, 211, 202, 201, 202, 211, 212, 212, 204, 202, 204, 212, 213, 213, 206, 204, 206, 213, 214, 214, 208, 206, 210, 215, 216, 216, 211, 210, 211, 216, 217, 217, 212, 211, 212, 217, 218, 218, 213, 212, 213, 218, 219, 219, 214, 213, 215, 220, 221, 221, 216, 215, 216, 221, 222, 222, 217, 216, 217, 222, 223, 223, 218, 217, 218, 223, 224, 224, 219, 218, 225, 226, 227, 227, 228, 225, 228, 227, 229, 229, 230, 228, 230, 229, 231, 231, 232, 230, 232, 231, 233, 233, 234, 232, 226, 235, 236, 236, 227, 226, 227, 236, 237, 237, 229, 227, 229, 237, 238, 238, 231, 229, 231, 238, 239, 239, 233, 231, 235, 240, 241, 241, 236, 235, 236, 241, 242, 242, 237, 236, 237, 242, 243, 243, 238, 237, 238, 243, 244, 244, 239, 238, 240, 245, 246, 246, 241, 240, 241, 246, 247, 247, 242, 241, 242, 247, 248, 248, 243, 242, 243, 248, 249, 249, 244, 243, 250, 251, 252, 252, 253, 250, 253, 252, 254, 254, 255, 253, 255, 254, 256, 256, 257, 255, 257, 256, 258, 258, 259, 257, 251, 260, 261, 261, 252, 251, 252, 261, 262, 262, 254, 252, 254, 262, 263, 263, 256, 254, 256, 263, 264, 264, 258, 256, 260, 265, 266, 266, 261, 260, 261, 266, 267, 267, 262, 261, 262, 267, 268, 268, 263, 262, 263, 268, 269, 269, 264, 263, 265, 270, 271, 271, 266, 265, 266, 271, 272, 272, 267, 266, 267, 272, 273, 273, 268, 267, 268, 273, 274, 274, 269, 268, 275, 276, 277, 277, 278, 275, 278, 277, 279, 279, 280, 278, 280, 279, 281, 281, 282, 280, 282, 281, 283, 283, 284, 282, 276, 285, 286, 286, 277, 276, 277, 286, 287, 287, 279, 277, 279, 287, 288, 288, 281, 279, 281, 288, 289, 289, 283, 281, 285, 290, 291, 291, 286, 285, 286, 291, 292, 292, 287, 286, 287, 292, 293, 293, 288, 287, 288, 293, 294, 294, 289, 288, 290, 295, 296, 296, 291, 290, 291, 296, 297, 297, 292, 291, 292, 297, 298, 298, 293, 292, 293, 298, 299, 299, 294, 293, 300, 301, 302, 302, 303, 300, 303, 302, 304, 304, 305, 303, 305, 304, 306, 306, 307, 305, 307, 306, 308, 308, 309, 307, 301, 310, 311, 311, 302, 301, 302, 311, 312, 312, 304, 302, 304, 312, 313, 313, 306, 304, 306, 313, 314, 314, 308, 306, 310, 315, 316, 316, 311, 310, 311, 316, 317, 317, 312, 311, 312, 317, 318, 318, 313, 312, 313, 318, 319, 319, 314, 313, 315, 320, 321, 321, 316, 315, 316, 321, 322, 322, 317, 316, 317, 322, 323, 323, 318, 317, 318, 323, 324, 324, 319, 318, 325, 326, 327, 327, 328, 325, 328, 327, 329, 329, 330, 328, 330, 329, 331, 331, 332, 330, 332, 331, 333, 333, 334, 332, 326, 335, 336, 336, 327, 326, 327, 336, 337, 337, 329, 327, 329, 337, 338, 338, 331, 329, 331, 338, 339, 339, 333, 331, 335, 340, 341, 341, 336, 335, 336, 341, 342, 342, 337, 336, 337, 342, 343, 343, 338, 337, 338, 343, 344, 344, 339, 338, 340, 345, 346, 346, 341, 340, 341, 346, 347, 347, 342, 341, 342, 347, 348, 348, 343, 342, 343, 348, 349, 349, 344, 343, 350, 351, 352, 352, 353, 350, 353, 352, 354, 354, 355, 353, 355, 354, 356, 356, 357, 355, 357, 356, 358, 358, 359, 357, 351, 360, 361, 361, 352, 351, 352, 361, 362, 362, 354, 352, 354, 362, 363, 363, 356, 354, 356, 363, 364, 364, 358, 356, 360, 365, 366, 366, 361, 360, 361, 366, 367, 367, 362, 361, 362, 367, 368, 368, 363, 362, 363, 368, 369, 369, 364, 363, 365, 370, 371, 371, 366, 365, 366, 371, 372, 372, 367, 366, 367, 372, 373, 373, 368, 367, 368, 373, 374, 374, 369, 368, 375, 376, 377, 377, 378, 375, 378, 377, 379, 379, 380, 378, 380, 379, 381, 381, 382, 380, 382, 381, 383, 383, 384, 382, 376, 385, 386, 386, 377, 376, 377, 386, 387, 387, 379, 377, 379, 387, 388, 388, 381, 379, 381, 388, 389, 389, 383, 381, 385, 390, 391, 391, 386, 385, 386, 391, 392, 392, 387, 386, 387, 392, 393, 393, 388, 387, 388, 393, 394, 394, 389, 388, 390, 395, 396, 396, 391, 390, 391, 396, 397, 397, 392, 391, 392, 397, 398, 398, 393, 392, 393, 398, 399, 399, 394, 393, 400, 401, 402, 402, 403, 400, 403, 402, 404, 404, 405, 403, 405, 404, 406, 406, 407, 405, 407, 406, 408, 408, 409, 407, 401, 410, 411, 411, 402, 401, 402, 411, 412, 412, 404, 402, 404, 412, 413, 413, 406, 404, 406, 413, 414, 414, 408, 406, 410, 415, 416, 416, 411, 410, 411, 416, 417, 417, 412, 411, 412, 417, 418, 418, 413, 412, 413, 418, 419, 419, 414, 413, 415, 420, 421, 421, 416, 415, 416, 421, 422, 422, 417, 416, 417, 422, 423, 423, 418, 417, 418, 423, 424, 424, 419, 418, 425, 426, 427, 427, 428, 425, 428, 427, 429, 429, 430, 428, 430, 429, 431, 431, 432, 430, 432, 431, 433, 433, 434, 432, 426, 435, 436, 436, 427, 426, 427, 436, 437, 437, 429, 427, 429, 437, 438, 438, 431, 429, 431, 438, 439, 439, 433, 431, 435, 440, 441, 441, 436, 435, 436, 441, 442, 442, 437, 436, 437, 442, 443, 443, 438, 437, 438, 443, 444, 444, 439, 438, 440, 445, 446, 446, 441, 440, 441, 446, 447, 447, 442, 441, 442, 447, 448, 448, 443, 442, 443, 448, 449, 449, 444, 443, 450, 451, 452, 452, 453, 450, 453, 452, 454, 454, 455, 453, 455, 454, 456, 456, 457, 455, 457, 456, 458, 458, 459, 457, 451, 460, 461, 461, 452, 451, 452, 461, 462, 462, 454, 452, 454, 462, 463, 463, 456, 454, 456, 463, 464, 464, 458, 456, 460, 465, 466, 466, 461, 460, 461, 466, 467, 467, 462, 461, 462, 467, 468, 468, 463, 462, 463, 468, 469, 469, 464, 463, 465, 470, 471, 471, 466, 465, 466, 471, 472, 472, 467, 466, 467, 472, 473, 473, 468, 467, 468, 473, 474, 474, 469, 468, 475, 476, 477, 477, 478, 475, 478, 477, 479, 479, 480, 478, 480, 479, 481, 481, 482, 480, 482, 481, 483, 483, 484, 482, 476, 485, 486, 486, 477, 476, 477, 486, 487, 487, 479, 477, 479, 487, 488, 488, 481, 479, 481, 488, 489, 489, 483, 481, 485, 490, 491, 491, 486, 485, 486, 491, 492, 492, 487, 486, 487, 492, 493, 493, 488, 487, 488, 493, 494, 494, 489, 488, 490, 495, 496, 496, 491, 490, 491, 496, 497, 497, 492, 491, 492, 497, 498, 498, 493, 492, 493, 498, 499, 499, 494, 493, 500, 501, 502, 502, 503, 500, 503, 502, 504, 504, 505, 503, 505, 504, 506, 506, 507, 505, 507, 506, 508, 508, 509, 507, 501, 510, 511, 511, 502, 501, 502, 511, 512, 512, 504, 502, 504, 512, 513, 513, 506, 504, 506, 513, 514, 514, 508, 506, 510, 515, 516, 516, 511, 510, 511, 516, 517, 517, 512, 511, 512, 517, 518, 518, 513, 512, 513, 518, 519, 519, 514, 513, 515, 520, 521, 521, 516, 515, 516, 521, 522, 522, 517, 516, 517, 522, 523, 523, 518, 517, 518, 523, 524, 524, 519, 518, 525, 526, 527, 527, 528, 525, 528, 527, 529, 529, 530, 528, 530, 529, 531, 531, 532, 530, 532, 531, 533, 533, 534, 532, 526, 535, 536, 536, 527, 526, 527, 536, 537, 537, 529, 527, 529, 537, 538, 538, 531, 529, 531, 538, 539, 539, 533, 531, 535, 540, 541, 541, 536, 535, 536, 541, 542, 542, 537, 536, 537, 542, 543, 543, 538, 537, 538, 543, 544, 544, 539, 538, 540, 545, 546, 546, 541, 540, 541, 546, 547, 547, 542, 541, 542, 547, 548, 548, 543, 542, 543, 548, 549, 549, 544, 543, 550, 551, 552, 552, 553, 550, 553, 552, 554, 554, 555, 553, 555, 554, 556, 556, 557, 555, 557, 556, 558, 558, 559, 557, 551, 560, 561, 561, 552, 551, 552, 561, 562, 562, 554, 552, 554, 562, 563, 563, 556, 554, 556, 563, 564, 564, 558, 556, 560, 565, 566, 566, 561, 560, 561, 566, 567, 567, 562, 561, 562, 567, 568, 568, 563, 562, 563, 568, 569, 569, 564, 563, 565, 570, 571, 571, 566, 565, 566, 571, 572, 572, 567, 566, 567, 572, 573, 573, 568, 567, 568, 573, 574, 574, 569, 568, 575, 576, 577, 577, 578, 575, 578, 577, 579, 579, 580, 578, 580, 579, 581, 581, 582, 580, 582, 581, 583, 583, 584, 582, 576, 585, 586, 586, 577, 576, 577, 586, 587, 587, 579, 577, 579, 587, 588, 588, 581, 579, 581, 588, 589, 589, 583, 581, 585, 590, 591, 591, 586, 585, 586, 591, 592, 592, 587, 586, 587, 592, 593, 593, 588, 587, 588, 593, 594, 594, 589, 588, 590, 595, 596, 596, 591, 590, 591, 596, 597, 597, 592, 591, 592, 597, 598, 598, 593, 592, 593, 598, 599, 599, 594, 593, 600, 601, 602, 602, 603, 600, 603, 602, 604, 604, 605, 603, 605, 604, 606, 606, 607, 605, 607, 606, 608, 608, 609, 607, 601, 610, 611, 611, 602, 601, 602, 611, 612, 612, 604, 602, 604, 612, 613, 613, 606, 604, 606, 613, 614, 614, 608, 606, 610, 615, 616, 616, 611, 610, 611, 616, 617, 617, 612, 611, 612, 617, 618, 618, 613, 612, 613, 618, 619, 619, 614, 613, 615, 620, 621, 621, 616, 615, 616, 621, 622, 622, 617, 616, 617, 622, 623, 623, 618, 617, 618, 623, 624, 624, 619, 618, 625, 626, 627, 627, 628, 625, 628, 627, 629, 629, 630, 628, 630, 629, 631, 631, 632, 630, 632, 631, 633, 633, 634, 632, 626, 635, 636, 636, 627, 626, 627, 636, 637, 637, 629, 627, 629, 637, 638, 638, 631, 629, 631, 638, 639, 639, 633, 631, 635, 640, 641, 641, 636, 635, 636, 641, 642, 642, 637, 636, 637, 642, 643, 643, 638, 637, 638, 643, 644, 644, 639, 638, 640, 645, 646, 646, 641, 640, 641, 646, 647, 647, 642, 641, 642, 647, 648, 648, 643, 642, 643, 648, 649, 649, 644, 643, 650, 651, 652, 652, 653, 650, 653, 652, 654, 654, 655, 653, 655, 654, 656, 656, 657, 655, 657, 656, 658, 658, 659, 657, 651, 660, 661, 661, 652, 651, 652, 661, 662, 662, 654, 652, 654, 662, 663, 663, 656, 654, 656, 663, 664, 664, 658, 656, 660, 665, 666, 666, 661, 660, 661, 666, 667, 667, 662, 661, 662, 667, 668, 668, 663, 662, 663, 668, 669, 669, 664, 663, 665, 670, 671, 671, 666, 665, 666, 671, 672, 672, 667, 666, 667, 672, 673, 673, 668, 667, 668, 673, 674, 674, 669, 668, 675, 676, 677, 677, 678, 675, 678, 677, 679, 679, 680, 678, 680, 679, 681, 681, 682, 680, 682, 681, 683, 683, 684, 682, 676, 685, 686, 686, 677, 676, 677, 686, 687, 687, 679, 677, 679, 687, 688, 688, 681, 679, 681, 688, 689, 689, 683, 681, 685, 690, 691, 691, 686, 685, 686, 691, 692, 692, 687, 686, 687, 692, 693, 693, 688, 687, 688, 693, 694, 694, 689, 688, 690, 695, 696, 696, 691, 690, 691, 696, 697, 697, 692, 691, 692, 697, 698, 698, 693, 692, 693, 698, 699, 699, 694, 693, 700, 701, 702, 702, 703, 700, 703, 702, 704, 704, 705, 703, 705, 704, 706, 706, 707, 705, 707, 706, 708, 708, 709, 707, 701, 710, 711, 711, 702, 701, 702, 711, 712, 712, 704, 702, 704, 712, 713, 713, 706, 704, 706, 713, 714, 714, 708, 706, 710, 715, 716, 716, 711, 710, 711, 716, 717, 717, 712, 711, 712, 717, 718, 718, 713, 712, 713, 718, 719, 719, 714, 713, 715, 720, 721, 721, 716, 715, 716, 721, 722, 722, 717, 716, 717, 722, 723, 723, 718, 717, 718, 723, 724, 724, 719, 718, 725, 726, 727, 727, 728, 725, 728, 727, 729, 729, 730, 728, 730, 729, 731, 731, 732, 730, 732, 731, 733, 733, 734, 732, 726, 735, 736, 736, 727, 726, 727, 736, 737, 737, 729, 727, 729, 737, 738, 738, 731, 729, 731, 738, 739, 739, 733, 731, 735, 740, 741, 741, 736, 735, 736, 741, 742, 742, 737, 736, 737, 742, 743, 743, 738, 737, 738, 743, 744, 744, 739, 738, 740, 745, 746, 746, 741, 740, 741, 746, 747, 747, 742, 741, 742, 747, 748, 748, 743, 742, 743, 748, 749, 749, 744, 743, 750, 751, 752, 752, 753, 750, 753, 752, 754, 754, 755, 753, 755, 754, 756, 756, 757, 755, 757, 756, 758, 758, 759, 757, 751, 760, 761, 761, 752, 751, 752, 761, 762, 762, 754, 752, 754, 762, 763, 763, 756, 754, 756, 763, 764, 764, 758, 756, 760, 765, 766, 766, 761, 760, 761, 766, 767, 767, 762, 761, 762, 767, 768, 768, 763, 762, 763, 768, 769, 769, 764, 763, 765, 770, 771, 771, 766, 765, 766, 771, 772, 772, 767, 766, 767, 772, 773, 773, 768, 767, 768, 773, 774, 774, 769, 768, 775, 776, 777, 777, 778, 775, 778, 777, 779, 779, 780, 778, 780, 779, 781, 781, 782, 780, 782, 781, 783, 783, 784, 782, 776, 785, 786, 786, 777, 776, 777, 786, 787, 787, 779, 777, 779, 787, 788, 788, 781, 779, 781, 788, 789, 789, 783, 781, 785, 790, 791, 791, 786, 785, 786, 791, 792, 792, 787, 786, 787, 792, 793, 793, 788, 787, 788, 793, 794, 794, 789, 788, 790, 795, 796, 796, 791, 790, 791, 796, 797, 797, 792, 791, 792, 797, 798, 798, 793, 792, 793, 798, 799, 799, 794, 793
  };
  int nIndices = sizeof(indices) / sizeof(GLuint);
  int nTriangles = nIndices / 3;

  GLfloat* verticesFlat = new GLfloat[9 * nTriangles];
  GLfloat* normalsFlat = new GLfloat[9 * nTriangles];
  GLfloat* texCoordsFlat = new GLfloat[9 * nTriangles];

  for (int i = 0; i < nTriangles; ++i) {
    for (int j = 0; j < 3; ++j) {     // 3 vertices per triangle
      for (int k = 0; k < 3; ++k) {   // 3 coordinates per vertex
        verticesFlat[3 * (3 * i + j) + k] = vertices[3 * indices[3 * i + j] + k];
        texCoordsFlat[3 * (3 * i + j) + k] = texCoords[3 * indices[3 * i + j] + k];
      }
    }
    // compute normal
    glm::vec3 v1 = glm::make_vec3(&(verticesFlat[3 * (3 * i + 0)]));
    glm::vec3 v2 = glm::make_vec3(&(verticesFlat[3 * (3 * i + 1)]));
    glm::vec3 v3 = glm::make_vec3(&(verticesFlat[3 * (3 * i + 2)]));
    glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);
    if (glm::length(normal) > FLT_EPSILON) {    // model contains degenerate traingles
      glm::normalize(normal);
    }
    for (int j = 0; j < 3; ++j) {     // 3 normals per triangle
      normalsFlat[3 * (3 * i + j) + 0] = normal.x;
      normalsFlat[3 * (3 * i + j) + 1] = normal.y;
      normalsFlat[3 * (3 * i + j) + 2] = normal.z;
    }
  }

  core->addAttributeData(OGLConstants::VERTEX.location, verticesFlat,
      9 * nTriangles * sizeof(GLfloat), 3, GL_STATIC_DRAW);

  core->addAttributeData(OGLConstants::NORMAL.location, normalsFlat,
      9 * nTriangles * sizeof(GLfloat), 3, GL_STATIC_DRAW);

  core->addAttributeData(OGLConstants::TEX_COORD_0.location, texCoordsFlat,
      9 * nTriangles * sizeof(GLfloat), 3, GL_STATIC_DRAW);

  delete [] verticesFlat;
  verticesFlat = nullptr;
  delete [] normalsFlat;
  normalsFlat = nullptr;
  delete [] texCoordsFlat;
  texCoordsFlat = nullptr;

  return core;
}


GeometryCoreSP GeometryCoreFactory::createXYZAxes(GLfloat size) {
  // create geometry core
  auto core = GeometryCore::create(GL_LINES, DrawMode::ARRAYS);

  // define vertices (6 vertices for 3 axes)
  GLfloat negSize = 0.2f * size;
  GLfloat posSize = 0.8f * size;
  const GLfloat vertices[] = {
      -negSize,  0.0f,     0.0f,   // x axis
       posSize,  0.0f,     0.0f,
       0.0f,    -negSize,  0.0f,     // y axis
       0.0f,     posSize,  0.0f,
       0.0f,     0.0f,    -negSize,  // z axis
       0.0f,     0.0f,     posSize
  };
  core->addAttributeData(OGLConstants::VERTEX.location, vertices,
      sizeof(vertices), 3, GL_STATIC_DRAW);

  // define vertex colors
  const GLfloat colors[] = {
      0.8f, 0.0f, 0.0f,
      0.8f, 0.0f, 0.0f,
      0.0f, 0.8f, 0.0f,
      0.0f, 0.8f, 0.0f,
      0.0f, 0.0f, 0.8f,
      0.0f, 0.0f, 0.8f
  };
  core->addAttributeData(OGLConstants::COLOR.location, colors,
      sizeof(colors), 3, GL_STATIC_DRAW);

  return core;
}


GeometryCoreSP GeometryCoreFactory::createRGBCube(GLfloat size) {
  // create geometry core
  auto core = GeometryCore::create(GL_TRIANGLES, DrawMode::ELEMENTS);

  // define vertices (cube volume = size x size x size,
  //   8 vertices with indices 0,...,7)
  //               7-----6
  //     y        /|    /|
  //     |       3-----2 |
  //     0-- x   | |   | |
  //    /        | 4---|-5
  //   z         |/    |/
  //             0-----1
  GLfloat halfSize = 0.5f * size;
  const GLfloat vertices[] = {
      -halfSize, -halfSize,  halfSize,
       halfSize, -halfSize,  halfSize,
       halfSize,  halfSize,  halfSize,
      -halfSize,  halfSize,  halfSize,
      -halfSize, -halfSize, -halfSize,
       halfSize, -halfSize, -halfSize,
       halfSize,  halfSize, -halfSize,
      -halfSize,  halfSize, -halfSize
  };
  core->addAttributeData(OGLConstants::VERTEX.location, vertices,
      sizeof(vertices), 3, GL_STATIC_DRAW);

  // define vertex colors
  const GLfloat colors[] = {
      0.0f, 0.0f,  1.0f,
      1.0f, 0.0f,  1.0f,
      1.0f, 1.0f,  1.0f,
      0.0f, 1.0f,  1.0f,
      0.0f, 0.0f,  0.0f,
      1.0f, 0.0f,  0.0f,
      1.0f, 1.0f,  0.0f,
      0.0f, 1.0f,  0.0f
  };
  core->addAttributeData(OGLConstants::COLOR.location, colors,
      sizeof(colors), 3, GL_STATIC_DRAW);

  // define indices (6 faces * 2 triangles -> 12 triangles)
  const GLuint indices[] = {
      0, 1, 3,  // front face
      1, 2, 3,
      5, 4, 6,  // rear face
      4, 7, 6,
      4, 0, 7,  // left face
      0, 3, 7,
      1, 5, 2,  // right face
      5, 6, 2,
      3, 2, 7,  // top face
      2, 6, 7,
      4, 5, 0,  // bottom face
      5, 1, 0
  };
  core->setElementIndexData(indices, sizeof(indices), GL_STATIC_DRAW);

  return core;
}


int GeometryCoreFactory::loadOBJFile_(const std::string& fileName, OBJModel& model) const {

  int error = 0;

  do {
    // try to find file
    std::string fullFileName = getFullFileName(filePaths_, fileName);
    if (fullFileName.empty()) {
      error = 1;
      break;
    }
    std::ifstream istr(fullFileName);
    if (!istr.is_open()) {
      error = 1;
      break;
    }

    // read OBJ file
    std::string line;
    while (std::getline(istr, line)) {
      if (line.empty()) {
        continue;
      }
      std::stringstream stream(line);
      std::string type;
      stream >> type;
      assert(stream.good() || stream.eof());
      if (type == "v") {
        glm::vec3 vertex;
        stream >> vertex.x >> vertex.y >> vertex.z;
        assert(stream.good() || stream.eof());
        model.vertices.push_back(vertex);
      }
      else if (type == "vt") {
        glm::vec2 texCoord;
        stream >> texCoord.s >> texCoord.t;
        assert(stream.good() || stream.eof());
        model.texCoords.push_back(texCoord);
      }
      else if (type == "vn") {
        glm::vec3 normal;
        stream >> normal.x >> normal.y >> normal.z;
        assert(stream.good() || stream.eof());
        model.normals.push_back(normal);
      }
      else if (type == "f") {
        Face face;
        do {
          FaceEntry entry;
          if (!(stream >> entry.vertex)) {
            // no more vertices in face
            break;
          }
          // check for texture coordinate and/or normal
          if (stream.peek() == '/') {
            stream.ignore(1);
            // check if texture coordinate is given
            if (stream.peek() != '/') {
              stream >> entry.texCoord;
              assert(stream.good() || stream.eof());
            }
            // check for normal
            if (stream.peek() == '/') {
              stream.ignore(1);
              stream >> entry.normal;
              assert(stream.good() || stream.eof());
            }
          }
          face.entries.push_back(entry);
        } while (true);
        assert(face.entries.size() >= 3);
        face.nTriangles = face.entries.size() - 2;
        model.nVertices += face.entries.size();
        model.nTriangles += face.nTriangles;
        model.faces.push_back(face);
      }
      // ignore other OBJ elements
    }
  } while (false);

  return error;
}


} /* namespace scg */
