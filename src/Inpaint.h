#ifndef IMAGESTACK_INPAINT_H
#define IMAGESTACK_INPAINT_H
#include "header.h"

class Inpaint : public Operation {
  public:
    void help();
    void parse(vector<string> args);
    static Image apply(Window im, Window mask);
};

#include "footer.h"
#endif
