/**
 * \file OpenGL3Viewer.cpp
 * \brief Simple OpenGL 3 viewer.
 *
 * Requires OpenGL 3.2, GLEW 1.13.0, and GLFW 3.2.0 (or later versions).
 *
 * \author Volker Ahlers\n
 *         volker.ahlers@hs-hannover.de
 */

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include "opengl_utils.h"  // provides loadShaders()
#include "OpenGL3Viewer.h"
//#include "RGBCubeModel.h"
//#include "TetraederModel.h"

#include "ZahnRadModel.h"


// --- public member functions ------------------------------------------------

static float winkel = 0.0;
static float winkelx = 0.0;
static float winkely = 0.0;
static float winkelz = 0.0;
static float achsex = 1.0;
static float achsey = 1.0;
static float achsez = 1.0;

OpenGL3Viewer::OpenGL3Viewer()
    : window_(nullptr), program_(0) {
}


OpenGL3Viewer::~OpenGL3Viewer() {
  if (glfwGetCurrentContext()) {    // check for OpenGL context
    glDeleteProgram(program_);
  }
  for (auto model : models_) {
    delete model;
    model = nullptr;
  }
  models_.clear();
  glfwTerminate();
  glfwSetErrorCallback(NULL);
}


void OpenGL3Viewer::init(const char* title, int width, int height) {

  // initialize GLFW
  glfwSetErrorCallback(errorCB_);
  int retCode = glfwInit();
  if (retCode == GL_FALSE) {
    throw std::runtime_error("glfwInit() failed [OpenGL3Viewer::init()]");
  }

  // open window and create OpenGL context
  //   OpenGL 3.2, forward-compatible, core profile
  //   8*4 bit RGBA color buffer
  //   24 bit depth buffer
  //   8 bit stencil buffer
  //   window mode (no fullscreen)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window_ = glfwCreateWindow(width, height, title, 0, 0);
  if (!window_) {
    throw std::runtime_error("glfwCreateWindow() failed [OpenGL3Viewer::init()]");
  }
  glfwMakeContextCurrent(window_);

  // synchronize frame rate with vertical display frequency
  glfwSwapInterval(1);

  // restrict aspect ratio
  glfwSetWindowAspectRatio(window_, 1, 1);

  assert(!checkGLError_());

  // initialize GLEW
  if (std::stoi(std::string(reinterpret_cast<const char*>(glewGetString(GLEW_VERSION_MAJOR)))) < 2) {
    glewExperimental = GL_TRUE;   // required for glGenVertexArrays() with GLEW versions < 2.0
  }
  GLenum error = glewInit();
  if (error != GLEW_OK) {
    throw std::runtime_error(std::string("GLEW error: ")
        + (const char*) glewGetErrorString(error) + " [OpenGL3Viewer::init()]");
  }
  glGetError();   // ignore GL error (invalid enum) in glewInit

  // print version information
  std::cout << "GL version: " << (const char*) glGetString(GL_VERSION) << std::endl;
  std::cout << "GL vendor: " << (const char*) glGetString(GL_VENDOR) << std::endl;
  std::cout << "GL renderer: " << (const char*) glGetString(GL_RENDERER) << std::endl;
  std::cout << "GLSL version: " << (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION)
      << std::endl;
  std::cout << "GLEW version: " << glewGetString(GLEW_VERSION) << std::endl;
  int major, minor, revision;
  glfwGetVersion(&major, &minor, &revision);
  std::cout << "GLFW version: " << major << "." << minor << "." << revision << std::endl;
  std::cout << "GLM version: " << GLM_VERSION_MAJOR << "." << GLM_VERSION_MINOR << "."
      << GLM_VERSION_PATCH << "." << GLM_VERSION_REVISION << std::endl << std::endl;

  // initialize shaders and models
  initShaders_();
  initModels_();

  // initialize projection matrix
  projection_ = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
  std::cout << "Projection: " << glm::to_string(projection_) << std::endl << std::endl;

  // print usage information
  std::cout << "Usage:" << std::endl
      << "  l - toggle line/fill mode" << std::endl << std::endl;
}


void OpenGL3Viewer::startMainLoop() {
  // main loop
	while (!glfwWindowShouldClose(window_)) {
    // initialize model-view matrix to identity
    //modelView_ = glm::mat4(1.0f);
    //modelView_ = translate(glm::mat4(1.0f), glm::vec3(0.5f,0.5f,0.5f));
	modelView_ = rotate(glm::mat4(1.0f), glm::radians(winkel), glm::vec3(achsex, achsey, achsez));
	//modelView_ = rotate(glm::mat4(1.0f), glm::radians(winkel), glm::vec3(0.0f, 0.0f, 1.0f));

	//modelView_ = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f,0.5f,0.5f));
    // check input
    checkKeyboard_();

    // clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // pass model-view-projection matrix to vertex shader
    glUniformMatrix4fv(glGetUniformLocation(program_, "mvpMatrix"), 1, GL_FALSE, glm::value_ptr(projection_ * modelView_));

    // render models
    for (auto model : models_) {
      model->render();
    }
    // models_[0]->render();    // alternative access to single models

    // swap front and back buffers
    glfwSwapBuffers(window_);
    glfwPollEvents();

    assert(!checkGLError_());
  }
}


// --- protected member functions ---------------------------------------------


void OpenGL3Viewer::initShaders_() {

  // load shaders
  program_ = glCreateProgram();
  glBindAttribLocation(program_, Model::ATTRIB_LOC_VERTEX, "vVertex");
  glBindAttribLocation(program_, Model::ATTRIB_LOC_NORMAL, "vNormal");
  glBindAttribLocation(program_, Model::ATTRIB_LOC_COLOR, "vColor");
  loadShaders(program_, "vertex.glsl", "fragment.glsl");
  glUseProgram(program_);

  // set OpenGL parameters
  glEnable(GL_DEPTH_TEST);                  // depth buffer test
  glClearColor(0.15f, 0.15f, 0.15f, 1.0f);  // background color

  assert(!checkGLError_());
}


void OpenGL3Viewer::initModels_() {

  // add models
  //models_.push_back(new(RGBCubeModel));
	//models_.push_back(new(TetraederModel));
	models_.push_back(new ZahnRadModel(0.5, 0.42, 0.1, 14.0, 16.0));
	models_.push_back(new ZahnRadModel(0.9, 0.2, 0.1, 14.0, 16.0));
}


void OpenGL3Viewer::checkKeyboard_() {

  // toggle line/fill mode
  static bool toggleKeyL = false;

  if(glfwGetKey(window_, GLFW_KEY_R) == GLFW_PRESS){
	  winkel = winkelz++;
	  achsey = 0.0;
	  achsex = 0.0;
	  achsez = 1.0;
  }
  if (glfwGetKey(window_, GLFW_KEY_R) == GLFW_RELEASE) {

  }

  if(glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS){
	  winkel = winkely++;
	  achsey = 0.0;
	  achsex = 1.0;
	  achsez = 0.0;
  }
  if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_RELEASE) {

  }
  if(glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS){
	  winkel = winkelx--;
	  achsey = 0.0;
	  achsex = 1.0;
	  achsez = 0.0;
  }
  if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_RELEASE) {

  }


  if(glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS){
	  winkel = winkely++;
	  achsey = 1.0;
	  achsex = 0.0;
	  achsez = 0.0;
  }
  if (glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_RELEASE) {

  }
  if(glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_PRESS){
	  winkel = winkely--;
	  achsey = 1.0;
	  achsex = 0.0;
	  achsez = 0.0;
  }
  if (glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_RELEASE) {

  }


  if (glfwGetKey(window_, GLFW_KEY_L) == GLFW_PRESS && !toggleKeyL) {
    static bool isLineMode = false;
    isLineMode = !isLineMode;
    isLineMode ?
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) :
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    toggleKeyL = true;
  }
  if (glfwGetKey(window_, GLFW_KEY_L) == GLFW_RELEASE) {
    toggleKeyL = false;
  }

  assert(!checkGLError_());
}


void OpenGL3Viewer::errorCB_(int error, const char* description) {
  std::cerr << "GLFW error: " << description << std::endl;
}
