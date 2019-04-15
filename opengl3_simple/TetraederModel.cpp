#include <cassert>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "opengl_utils.h"
#include <math.h>
#include "TetraederModel.h"


// --- public member functions ------------------------------------------------


TetraederModel::TetraederModel() {
  init_();
}


TetraederModel::~TetraederModel() {
  if (glfwGetCurrentContext()) {    // check for OpenGL context
    glDeleteVertexArrays(1, &vao_);
  }
}


void TetraederModel::render() const {
  // render model using vertex array object
  glBindVertexArray(vao_);
  glDrawElements(GL_TRIANGLES, nIndices_, GL_UNSIGNED_INT, static_cast<const GLvoid*>(0));
  glBindVertexArray(0);
}


// --- protected member functions ------------------------------------------------


void TetraederModel::init_() {
  // create and bind vertex array;
  // in case of multiple objects, use a separate vertex array for each one
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);
  assert(glIsVertexArray(vao_));

   const GLfloat tetraVertices[] = {
	0.0f, (sqrt(6)/3.0)/2, 0.0f,
	-0.5f, -(sqrt(6)/3.0)/2, -(sqrt(0.75)/2),
	0.5f, -(sqrt(6)/3.0)/2, -(sqrt(0.75)/2),
	0.0f, -(sqrt(6)/3.0)/2, 0.5f
	};

  // create vertex VBO and enable vertex attribute
  GLuint vboVertex;
  glGenBuffers(1, &vboVertex);
  glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
  assert(glIsBuffer(vboVertex));
  glBufferData(GL_ARRAY_BUFFER, sizeof(tetraVertices), tetraVertices, GL_STATIC_DRAW);
  glVertexAttribPointer(ATTRIB_LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, static_cast<const GLvoid*>(0));
  glEnableVertexAttribArray(ATTRIB_LOC_VERTEX);

  // cube vertex colors
  const GLfloat tetraColors[] = {
      0.0f, 0.0f,  1.0f,
      1.0f, 0.0f,  1.0f,
      1.0f, 1.0f,  0.0f,
      0.0f, 1.0f,  0.0f };

  // create color VBO and enable color attribute
  GLuint vboColor;
  glGenBuffers(1, &vboColor);
  glBindBuffer(GL_ARRAY_BUFFER, vboColor);
  assert(glIsBuffer(vboColor));
  glBufferData(GL_ARRAY_BUFFER, sizeof(tetraColors), tetraColors, GL_STATIC_DRAW);
  glVertexAttribPointer(ATTRIB_LOC_COLOR, 3, GL_FLOAT, GL_FALSE, 0, static_cast<const GLvoid*>(0));
  glEnableVertexAttribArray(ATTRIB_LOC_COLOR);

  // cube indices (6 faces * 2 triangles -> 12 triangles)
  const GLuint tetraIndices[] = {
      0, 1, 2,
      0, 1, 3,
      0, 3, 2,
      1, 2, 3 };

  // create index VBO
  GLuint vboIndex;
  glGenBuffers(1, &vboIndex);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndex);
  assert(glIsBuffer(vboIndex));
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tetraIndices), tetraIndices, GL_STATIC_DRAW);
  nIndices_ = sizeof(tetraIndices) / sizeof(GLuint);

  // unbind vertex array
  glBindVertexArray(0);

  assert(!checkGLError_());
}
