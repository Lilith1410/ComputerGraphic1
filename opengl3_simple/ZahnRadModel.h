
#ifndef ZAHNRADMODEL_H_
#define ZAHNRADMODEL_H_

#include "Model.h"


class ZahnRadModel : public Model {

public:

  ZahnRadModel(double, double, double, double, double);

  virtual ~ZahnRadModel();

  virtual void render() const;

protected:

  void init_(double, double, double, double, double);

};


#endif /* ZAHNRADMODEL_H_ */
