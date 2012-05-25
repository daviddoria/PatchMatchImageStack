#ifndef IMAGESTACK_BidirectionalSimilarity_H
#define IMAGESTACK_BidirectionalSimilarity_H
#include "header.h"

class BidirectionalSimilarity : public Operation
{
  public:
    void help();
    void parse(vector<string> args);
    static void apply(Window source, Window target,
                      Window sourceMask, Window targetMask,
                      int numIter, int numIterPM = 5);

};


class Heal : public Operation
{
  public:
    void help();
    void parse(vector<string> args);
};

#include "footer.h"
#endif
