/**
 * \file opengl_utils.h
 * \brief Utilities to load shaders and check for OpenGL errors.
 *
 * \author Volker Ahlers\n
 *         volker.ahlers@hs-hannover.de
 */


#ifndef OGL_UTILS_H_
#define OGL_UTILS_H_


#include <string>
#include <GL/glew.h>


/**
 * \brief Check for OpenGL error.
 * \return 0 for okay, 1 for error
 */
int checkGLError_();


/**
 * \brief Load vertex and fragment shaders from files, compile, create program, link.
 * \param program ID
 * \param vertexShaderFileName
 * \param fragmentShaderFileName
 */
void loadShaders(GLuint program, std::string vertexShaderFileName, std::string fragmentShaderFileName);


/**
 * \brief Load shader from file.
 *
 * Low-level function called by loadShaders().
 * \param shaderID valid shader ID
 * \param fileName
 */
void loadShaderFile(GLuint shaderID, const std::string& fileName);


/**
 * \brief Check for OpenGL error after compiling shader.
 *
 * Low-level function called by loadShaders().
 * \param shaderID valid shader ID
 */
void checkGLCompileError(GLuint shaderID);


/**
 * \brief Check for OpenGL error after linking program.
 *
 * Low-level function called by loadShaders().
 * \param programID valid program ID
 */
void checkGLLinkError(GLuint programID);


#endif /* OGL_UTILS_H_ */
