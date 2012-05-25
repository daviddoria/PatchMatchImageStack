#ifndef IMAGESTACK_BidirectionalSimilarity_H
#define IMAGESTACK_BidirectionalSimilarity_H
#include "header.h"

class Heal : public Operation
{
  public:
    void help();
    void parse(vector<string> args);
    static void apply(Image image, Image mask,
                      int numIter = 5, int numIterPM = 5, int patchDiameter = 15);
};

#include "footer.h"
#endif
