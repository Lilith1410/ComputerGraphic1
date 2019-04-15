/**
 * \file RGBCubeModel.h
 * \brief RGB cube model without normals
 *
 * \author Volker Ahlers\n
 *         volker.ahlers@hs-hannover.de
 */

#ifndef RGBCUBEMODEL_H_
#define RGBCUBEMODEL_H_

#include "Model.h"


class RGBCubeModel : public Model {

public:

  RGBCubeModel();

  virtual ~RGBCubeModel();

  virtual void render() const;

protected:

  void init_();

};


#endif /* RGBCUBEMODEL_H_ */
