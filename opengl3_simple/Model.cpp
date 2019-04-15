/**
 * \file Model.cpp
 * \brief Abstract model base class
 *
 * \author Volker Ahlers\n
 *         volker.ahlers@hs-hannover.de
 */

#include "Model.h"


// --- public constants -------------------------------------------------------


const GLuint Model::ATTRIB_LOC_VERTEX = 0;
const GLuint Model::ATTRIB_LOC_NORMAL = 1;
const GLuint Model::ATTRIB_LOC_COLOR = 2;


// --- public member functions ------------------------------------------------


Model::Model()
    : vao_(0), nIndices_(0) {
}


Model::~Model() {
}
