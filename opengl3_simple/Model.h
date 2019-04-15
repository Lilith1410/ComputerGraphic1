/**
 * \file Model.h
 * \brief Abstract model base class
 *
 * \author Volker Ahlers\n
 *         volker.ahlers@hs-hannover.de
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <GL/glew.h>


class Model {

public:

  Model();

  virtual ~Model();

  virtual void render() const = 0;

public:

  // vertex attribute locations
  static const GLuint ATTRIB_LOC_VERTEX;
  static const GLuint ATTRIB_LOC_NORMAL;
  static const GLuint ATTRIB_LOC_COLOR;

protected:

  GLuint vao_;              // vertex array object
  GLsizei nIndices_;        // number of triangle indices

};


#endif /* MODEL_H_ */
