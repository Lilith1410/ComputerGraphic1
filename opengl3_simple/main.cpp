/**
 * \file main.cpp
 * \brief Simple self-contained OpenGL 3 application.
 *
 * Requires OpenGL 3.2, GLEW 1.10.0, and GLFW 3.2.0 (or later versions).
 *
 * \author Volker Ahlers\n
 *         volker.ahlers@hs-hannover.de
 */

#include <iostream>
#include <stdexcept>
#include "OpenGL3Viewer.h"


/**
 * \brief Main function using OpenGL3Viewer class.
 * \return 0 for success, 1 for error
 */
int main() {
  int result = 0;

  try {
    OpenGL3Viewer viewer;
    viewer.init("OpenGL3 Simple", 800, 800);
//    viewer.addModel(rgbCube);
    viewer.startMainLoop();
  }
  catch (const std::exception& exc) {
    std::cerr << "Exception: " << exc.what() << std::endl;
    result = 1;
  }

  return result;
}
