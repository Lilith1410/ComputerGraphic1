
#ifndef TETRAEDERMODEL_H_
#define TETRAEDERMODEL_H_

#include "Model.h"


class TetraederModel : public Model {

public:

  TetraederModel();

  virtual ~TetraederModel();

  virtual void render() const;

protected:

  void init_();

};


#endif /* TETRAEDERMODEL_H_ */
