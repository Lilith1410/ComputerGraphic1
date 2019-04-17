/**
 * \file opengl_utils.cpp
 * \brief Utilities to load shaders and check for OpenGL errors.
 *
 * \author Volker Ahlers\n
 *         volker.ahlers@hs-hannover.de
 */


#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "opengl_utils.h"


int checkGLError_() {
  int result = 0;
  std::map<GLenum, std::string> messages;
  messages[GL_INVALID_ENUM] = "invalid enum";
  messages[GL_INVALID_VALUE] = "invalid value";
  messages[GL_INVALID_OPERATION] = "invalid operation";
  messages[GL_STACK_OVERFLOW] = "stack overflow";
  messages[GL_STACK_UNDERFLOW] = "stack underflow";
  messages[GL_OUT_OF_MEMORY] = "out of memory";
  messages[GL_INVALID_FRAMEBUFFER_OPERATION] = "invalid framebuffer operation";

  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    std::cout << std::endl << "OpenGL error: " << messages[error] << std::endl;
    result = 1;
  }
  return result;
}


void loadShaders(GLuint program, std::string vertexShaderFileName, std::string fragmentShaderFileName) {

  // compile vertex shader
  GLuint shaderVert = glCreateShader(GL_VERTEX_SHADER);
  assert(glIsShader(shaderVert));
  loadShaderFile(shaderVert, vertexShaderFileName);
  glCompileShader(shaderVert);
  checkGLCompileError(shaderVert);

  // compile fragment shader
  GLuint shaderFrag = glCreateShader(GL_FRAGMENT_SHADER);
  assert(glIsShader(shaderFrag));
  loadShaderFile(shaderFrag, fragmentShaderFileName);
  glCompileShader(shaderFrag);
  checkGLCompileError(shaderFrag);

  // link shader program
  glAttachShader(program, shaderVert);
  glAttachShader(program, shaderFrag);
  glLinkProgram(program);
  checkGLLinkError(program);
}


void loadShaderFile(GLuint shaderID, const std::string& fileName) {

  // try to open file
  std::ifstream istr(fileName.c_str());
  if (!istr.is_open()) {
    std::cerr << std::endl << "Error: cannot open shader file " << fileName << std::endl;
    exit(1);
  }

  // read shader source
  char line[1024];
  std::vector<std::string> sourceVec;
  while (istr.getline(line, 1024)) {
    sourceVec.push_back(std::string(line) + '\n');
  }
  istr.close();

  // pass shader source to OpenGL
  const GLchar** source = new const GLchar*[sourceVec.size()];
  for (unsigned int i = 0; i < sourceVec.size(); i++) {
    source[i] = reinterpret_cast<const GLchar*>(sourceVec[i].c_str());
  }
  assert(glIsShader(shaderID));
  glShaderSource(shaderID, sourceVec.size(), source, NULL);
  delete[] source;
  source = 0;
}


void checkGLCompileError(GLuint shaderID) {
  GLint status = GL_TRUE;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    const size_t maxLogLength = 1000;
    GLchar infoLog[maxLogLength];
    glGetShaderInfoLog(shaderID, maxLogLength, NULL, infoLog);
    std::cerr << "Error: compiling shader " << shaderID << " failed"
        << std::endl << infoLog << std::endl;
    exit(1);
  }
}


void checkGLLinkError(GLuint programID) {
  GLint status = GL_TRUE;
  glGetProgramiv(programID, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    const size_t maxLogLength = 1000;
    GLchar infoLog[maxLogLength];
    glGetProgramInfoLog(programID, maxLogLength, NULL, infoLog);
    std::cerr << std::endl << "Error: linking program " << programID << " failed"
        << std::endl << infoLog << std::endl;
    exit(1);
  }
}
