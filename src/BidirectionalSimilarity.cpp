// CS448F final project
// Implementation of PatchMatch algorithm and its applications
// Sung Hee Park (shpark7@stanford.edu)

#include "main.h"
#include "File.h"
#include "Geometry.h"
#include "Inpaint.h"
#include "BidirectionalSimilarity.h"
#include "Arithmetic.h"
#include "Calculus.h"
#include "Statistics.h"
#include "Filter.h"
#include "Paint.h"
#include "PatchMatch.h"

#include "header.h"

void Heal::apply(Window image, Window mask,
                 int numIter, int numIterPM)
{
  // Smoothly fill the hole
  Inpaint::apply(image, mask);
  image.Write("image_smooth_filled.png");
  
  int patchSize = 15;

  for(int i = 0; i < numIter; i++)
  {
    std::cout << ".";

    // The homogeneous output for this iteration
    Image out(image.width, image.height, 1, image.channels+1); // +1 to store the weight?

    // COHERENCE TERM
    Image coherentMatch = PatchMatch::apply(image, image, mask,
                                            numIterPM, patchSize);

    // For every patch in the target, pull from the nearest match in the source
    float *matchPtr = coherentMatch(0, 0);

    // The contribution of each pixel q to the error term (d_cohere) = 1/N_T \sum_{i=1}^m (S(p_i) - T(q))^2
    // To find the best color T(q) (iterative update rule), differentiate with respect to T(q),
    // set to 0, and solve for T(q):
    // T(q) = \frac{1}{m} \sum_{i=1}^m S(p_i)

    // Loop over the whole image (patch centers)
    for (int y = 0; y < image.height; y++)
    {
      float *targMaskPtr = mask(0, y);
      for (int x = 0; x < image.width; x++)
      {
        if (!mask || targMaskPtr[0] > 0)
        {
          int dstX = (int)matchPtr[0];
          int dstY = (int)matchPtr[1];

          float weight = 1.0f/(matchPtr[2]+1);

          if (mask)
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
            float *sourcePtr = image(dstX-patchSize/2, dstY+dy);
            float *outPtr = out(x-patchSize/2, y+dy);
            for (int dx = -patchSize/2; dx <= patchSize/2; dx++)
            {
              if (x+dx < 0)
              {
                outPtr += out.channels;
                sourcePtr += image.channels;
              }
              else if (x+dx >= out.width)
              {
                break;
              }
              else
              {
                for (int c = 0; c < image.channels; c++)
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

  apply(image, mask, numIter, numIterPM);
}

#include "footer.h"
