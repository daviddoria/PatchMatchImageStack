#ifndef IMAGESTACK_PATCHMATCH_H
#define IMAGESTACK_PATCHMATCH_H
#include "header.h"

class PatchMatch : public Operation
{
public:

  void help();
  void parse(vector<string> args);

  static Image apply(Window source, Window target, Window mask, int iterations, int patchSize, Image initialization);

  static void Random(Image patchMatchImage, Image image, Image mask, const int patchSize);
  
private:

  static float distance(Window source, Window target, Window mask,
                        int sx, int sy,
                        int tx, int ty,
                        int patchSize, float prevDist);


};

#include "footer.h"
#endif
