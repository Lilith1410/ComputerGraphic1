/**
 * \file RGBCubeModel.cpp
 * \brief RGB cube model without normals
 *
 * \author Volker Ahlers\n
 *         volker.ahlers@hs-hannover.de
 */

#include <cassert>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "opengl_utils.h"
#include "RGBCubeModel.h"


// --- public member functions ------------------------------------------------


RGBCubeModel::RGBCubeModel() {
  init_();
}


RGBCubeModel::~RGBCubeModel() {
  if (glfwGetCurrentContext()) {    // check for OpenGL context
    glDeleteVertexArrays(1, &vao_);
  }
}


void RGBCubeModel::render() const {
  // render model using vertex array object
  glBindVertexArray(vao_);
  glDrawElements(GL_TRIANGLES, nIndices_, GL_UNSIGNED_INT, static_cast<const GLvoid*>(0));
  glBindVertexArray(0);
}


// --- protected member functions ------------------------------------------------


void RGBCubeModel::init_() {
  // create and bind vertex array;
  // in case of multiple objects, use a separate vertex array for each one
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);
  assert(glIsVertexArray(vao_));

  // cube vertices (cube volume = 1 x 1 x 1,
  //   8 vertices with indices 0,...,7)
  //               7-----6
  //     y        /|    /|
  //     |       3-----2 |
  //     0-- x   | |   | |
  //    /        | 4---|-5
  //   z         |/    |/
  //             0-----1
  const GLfloat cubeVertices[] = {
      -0.5f, -0.5f,  0.5f,
       0.5f, -0.5f,  0.5f,
       0.5f,  0.5f,  0.5f,
      -0.5f,  0.5f,  0.5f,
      -0.5f, -0.5f, -0.5f,
       0.5f, -0.5f, -0.5f,
       0.5f,  0.5f, -0.5f,
      -0.5f,  0.5f, -0.5f };

  // create vertex VBO and enable vertex attribute
  GLuint vboVertex;
  glGenBuffers(1, &vboVertex);
  glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
  assert(glIsBuffer(vboVertex));
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
  glVertexAttribPointer(ATTRIB_LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, static_cast<const GLvoid*>(0));
  glEnableVertexAttribArray(ATTRIB_LOC_VERTEX);

  // cube vertex colors
  const GLfloat cubeColors[] = {
      0.0f, 0.0f,  1.0f,
      1.0f, 0.0f,  1.0f,
      1.0f, 1.0f,  1.0f,
      0.0f, 1.0f,  1.0f,
      0.0f, 0.0f,  0.0f,
      1.0f, 0.0f,  0.0f,
      1.0f, 1.0f,  0.0f,
      0.0f, 1.0f,  0.0f };

  // create color VBO and enable color attribute
  GLuint vboColor;
  glGenBuffers(1, &vboColor);
  glBindBuffer(GL_ARRAY_BUFFER, vboColor);
  assert(glIsBuffer(vboColor));
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);
  glVertexAttribPointer(ATTRIB_LOC_COLOR, 3, GL_FLOAT, GL_FALSE, 0, static_cast<const GLvoid*>(0));
  glEnableVertexAttribArray(ATTRIB_LOC_COLOR);

  // cube indices (6 faces * 2 triangles -> 12 triangles)
  const GLuint cubeIndices[] = {
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
      5, 1, 0 };

  // create index VBO
  GLuint vboIndex;
  glGenBuffers(1, &vboIndex);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndex);
  assert(glIsBuffer(vboIndex));
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
  nIndices_ = sizeof(cubeIndices) / sizeof(GLuint);

  // unbind vertex array
  glBindVertexArray(0);

  assert(!checkGLError_());
}
