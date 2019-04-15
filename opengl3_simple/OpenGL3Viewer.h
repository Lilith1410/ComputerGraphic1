/**
 * \file OpenGL3Viewer.h
 * \brief Simple OpenGL 3 viewer.
 *
 * Requires OpenGL 3.2, GLEW 1.13.0, and GLFW 3.2.0 (or later versions).
 *
 * \author Volker Ahlers\n
 *         volker.ahlers@hs-hannover.de
 */

#ifndef OPENGL3VIEWER_H_
#define OPENGL3VIEWER_H_

#include <vector>
#include <GL/glew.h>                                // has to be included before glfw.h
#include <GLFW/glfw3.h>
#include "extern/glm/glm/glm.hpp"                   // provides mat4 matrix type
#include "extern/glm/glm/gtc/matrix_transform.hpp"  // provides translate(), rotate(), etc.
#include "extern/glm/glm/gtc/type_ptr.hpp"          // provides value_ptr()
#define GLM_ENABLE_EXPERIMENTAL
#include "extern/glm/glm/gtx/string_cast.hpp"       // provides experimental vector and matrix stream output
#include "Model.h"


/**
 * \brief Simple OpenGL 3 viewer.
 *
 * Static member functions are required for GLFW callbacks.
 */
class OpenGL3Viewer {

public:

  // constructor
  OpenGL3Viewer();

  // destructor
  virtual ~OpenGL3Viewer();

  // create window and OpenGL context, compile shaders, initialize model
  void init(const char* title, int width, int height);

  // main rendering loop: check for input, render scene
  void startMainLoop();

protected:

  // compile shaders
  void initShaders_();

  // add models
  void initModels_();

  // check for keyboard input, called by startMainLoop()
  void checkKeyboard_();

  // callback function for GLFW errors
  static void errorCB_(int error, const char* description);

protected:

  GLFWwindow* window_;          // application window
  GLuint program_;              // shader program
  std::vector<Model*> models_;  // model to be rendered
  glm::mat4 modelView_;         // model-view matrix
  glm::mat4 projection_;        // projection matrix

};


#endif /* OPENGL3VIEWER_H_ */
