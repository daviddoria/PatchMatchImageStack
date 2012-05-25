#ifndef IMAGESTACK_BidirectionalSimilarity_H
#define IMAGESTACK_BidirectionalSimilarity_H
#include "header.h"

class Heal : public Operation
{
  public:
    void help();
    void parse(vector<string> args);
    static void apply(Window image, Window mask,
                      int numIter, int numIterPM = 5);
};

#include "footer.h"
#endif
