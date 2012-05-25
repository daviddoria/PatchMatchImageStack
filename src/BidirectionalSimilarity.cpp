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

void Heal::apply(Window imageIn, Window mask,
                 int numIter, int numIterPM)
{
  imageIn.WritePNG("image.png");
  mask.WriteMask("mask.png");
  
  // Smoothly fill the hole
  std::cout << "Image has " << imageIn.channels << " channels." << std::endl;
  // Not sure why this doesn't work (when the input argument was named 'image')
  // so we simply renamed the argument and smooth-fill the image into the image called 'image'.

//   image = Inpaint::apply(image, mask);
//   std::cout << "Image has " << image.channels << " channels." << std::endl;
//   image.WritePNG("image_smooth_filled.png");

//   Image image = Inpaint::apply(imageIn, mask);
//   image.WritePNG("image_smooth_filled.png");

  Image image = imageIn;

  int patchDiameter = 15;

  Image out(image.width, image.height, 1, image.channels+1); // +1 to store the weight?
  out.Zero();
  
  for(int i = 0; i < numIter; i++)
  {
    std::cout << "Heal Iteration " << i << std::endl;

    // COHERENCE TERM
    Image coherentMatch = PatchMatch::apply(image, image, mask,
                                            numIterPM, patchDiameter);
    std::stringstream ssPatchMatch;
    ssPatchMatch << "Heal_PatchMatch_" << i << ".mha";
    coherentMatch.WriteMeta(ssPatchMatch.str());
    std::cout << "Finished PatchMatch." << std::endl;

    // For every patch in the target, pull from the nearest match in the source
    float *matchPtr = coherentMatch(0, 0);

    // The contribution of each pixel q to the error term (d_cohere) = 1/N_T \sum_{i=1}^m (S(p_i) - T(q))^2
    // To find the best color T(q) (iterative update rule), differentiate with respect to T(q),
    // set to 0, and solve for T(q):
    // T(q) = \frac{1}{m} \sum_{i=1}^m S(p_i)

    // Loop over the whole image (patch centers)
    for (int y = 0; y < image.height; y++)
    {
      float *maskPtr = mask(0, y);
      for (int x = 0; x < image.width; x++)
      {
        if (maskPtr[0] == 0) // Fill this pixel
        {
          int dstX = (int)matchPtr[0];
          int dstY = (int)matchPtr[1];

          // weight by the SSD score (need to normalize this if we want to use it)
          // float weight = 1.0f/(matchPtr[2]+1);

          // weight equally
          float weight = 1.0f/(patchDiameter*patchDiameter);

          for (int dy = -patchDiameter/2; dy <= patchDiameter/2; dy++)
          {
            if (y+dy < 0)// skip this row
            {
              continue;
            }
            if (y+dy >= out.height) // quit when we get past the last row
            {
              break;
            }
            float *imagePtr = image(dstX-patchDiameter/2, dstY+dy);
            float *outPtr = out(x-patchDiameter/2, y+dy);
            for (int dx = -patchDiameter/2; dx <= patchDiameter/2; dx++)
            {
              if (x+dx < 0) // skip this pixel
              {
                outPtr += out.channels;
                imagePtr += image.channels;
              }
              else if (x+dx >= out.width) // quit when we get to the end of the row
              {
                break;
              }
              else
              {
                for (int c = 0; c < image.channels; c++)
                {
                  (*outPtr++) += weight*(*imagePtr++);
                }
                (*outPtr++) += weight;
              }
            }
          }
        } // end if (!targetMask || targMaskPtr[0] > 0)
        else // Copy the input to the output
        {
          float *imagePtr = image(x, y);
          float *outPtr = out(x, y);
          for (int c = 0; c < image.channels; c++)
          {
            (*outPtr++) += (*imagePtr++);
          }
        }

        maskPtr++;
        matchPtr += coherentMatch.channels;
      }
    }

    std::stringstream ssMeta;
    ssMeta << "Iteration_" << i << ".mha";
    out.WriteMeta(ssMeta.str());

    std::stringstream ssPNG;
    ssPNG << "Iteration_" << i << ".png";
    out.WritePNG(ssPNG.str());

    // reset for the next iteration
    image = out;
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
  std::cout << "Mask has " << mask.channels << " channels." << std::endl;

  Window image = stack(0);
  std::cout << "Image has " << image.channels << " channels." << std::endl;

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
