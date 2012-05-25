// CS448F final project
// Implementation of PatchMatch algorithm and its applications
// Sung Hee Park (shpark7@stanford.edu)

#include "main.h"
#include "File.h"
#include "Geometry.h"
#include "BidirectionalSimilarity.h"
#include "Arithmetic.h"
#include "Calculus.h"
#include "Statistics.h"
#include "Filter.h"
#include "Paint.h"
#include "PatchMatch.h"
#include "Output.h"
#include "header.h"

void BidirectionalSimilarity::help()
{

}

void BidirectionalSimilarity::parse(vector<string> args)
{
  std::cerr << "Cannot run this directly." << std::endl;
}


// Reconstruct the portion of the target where the targetMask is high, using
// the portion of the source where the sourceMask is high.
void BidirectionalSimilarity::apply(Window source, Window target,
                                    Window sourceMask, Window targetMask,
                                    int numIter, int numIterPM)
{
  int patchSize = 15;

  for(int i = 0; i < numIter; i++)
  {
    std::cout << ".";

    // The homogeneous output for this iteration
    Image out(target.width, target.height, 1, target.channels+1); // +1 to store the weight?

    // COHERENCE TERM
    Image coherentMatch = PatchMatch::apply(target, source, sourceMask,
                                            numIterPM, patchSize);

    // For every patch in the target, pull from the nearest match in the source
    float *matchPtr = coherentMatch(0, 0);

    // Loop over the whole image (patch centers)
    for (int y = 0; y < target.height; y++)
    {
      float *targMaskPtr = targetMask(0, y);
      for (int x = 0; x < target.width; x++)
      {
        if (!targetMask || targMaskPtr[0] > 0)
        {
          int dstX = (int)matchPtr[0];
          int dstY = (int)matchPtr[1];

          float weight = 1.0f/(matchPtr[2]+1);

          if (targetMask)
          {
            weight *= targMaskPtr[0];
          }

          for (int dy = -patchSize/2; dy <= patchSize/2; dy++)
          {
            if (y+dy < 0)
            {
              continue;
            }
            if (y+dy >= out.height)
            {
              break;
            }
            float *sourcePtr = source(dstX-patchSize/2, dstY+dy);
            float *outPtr = out(x-patchSize/2, y+dy);
            for (int dx = -patchSize/2; dx <= patchSize/2; dx++)
            {
              if (x+dx < 0)
              {
                outPtr += out.channels;
                sourcePtr += source.channels;
              }
              else if (x+dx >= out.width)
              {
                break;
              }
              else
              {
                for (int c = 0; c < source.channels; c++)
                {
                  (*outPtr++) += weight*(*sourcePtr++);
                }
                (*outPtr++) += weight;
              }
            }
          }
        } // end if (!targetMask || targMaskPtr[0] > 0)

        targMaskPtr++;
        matchPtr += coherentMatch.channels;
      }
    }

    // rewrite the target
    float *outPtr = out(0, 0);
    float *targMaskPtr = targetMask(0, 0);
    for (int y = 0; y < out.height; y++)
    {
      float *targetPtr = target(0, y);
      for (int x = 0; x < out.width; x++)
      {
        float w = 1.0f/(outPtr[target.channels]);
        float a = 1;
        if (targetMask)
        {
          a = *targMaskPtr++;
        }
        if (a == 1)
        {
          for (int c = 0; c < target.channels; c++)
          {
            targetPtr[0] = w*(*outPtr++);
            targetPtr++;
          }
        }
        else if (a > 0)
        {
          for (int c = 0; c < target.channels; c++)
          {
            targetPtr[0] *= (1-a);
            targetPtr[0] += a*w*(*outPtr++);
            targetPtr++;
          }
        }
        else
        {
          targetPtr += target.channels;
          outPtr += target.channels;
        }
        outPtr++;
      } // end for x
    } // end for y

    std::stringstream ss;
    ss << "Iteration_" << i << ".mha";
    out.Write(ss.str());
  } // end for(int i = 0; i < numIter; i++)
  std::cout << std::endl;
}

void Heal::help()
{
    printf("-heal takes an image and a mask, and reconstructs the portion of"
           " the image where the mask is high using patches from the rest of the"
           " image. It uses the patchmatch algorithm for acceleration. The"
           " arguments include the number of iterations to run per scale, and the"
           " number of iterations of patchmatch to run. Both default to five.\n"
           "\n"
           "Usage: ImageStack -load mask.png -load image.jpg -heal -save out.png \n");
}

void Heal::parse(vector<string> args)
{
  int numIter = 5;
  int numIterPM = 5;

  assert(args.size() < 3, "-heal takes zero, one, or two arguments\n");

  Window mask = stack(1);
  Window image = stack(0);

  Image inverseMask(mask);
  Scale::apply(inverseMask, -1);
  Offset::apply(inverseMask, 1);

  if (args.size() > 0)
  {
    numIter = readInt(args[0]);
  }
  if (args.size() > 1)
  {
    numIterPM = readInt(args[1]);
  }

  BidirectionalSimilarity::apply(image, image, inverseMask, mask, numIter, numIterPM);
}

#include "footer.h"
