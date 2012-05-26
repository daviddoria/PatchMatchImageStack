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

#include "Mask/ITKHelpers/Helpers/Helpers.h"

#include "header.h"

void Heal::apply(Image imageIn, Image mask,
                 int numIter, int numIterPM, int patchDiameter)
{
  imageIn.WritePNG("image.png");
  mask.WriteMask("mask.png");

  // Smoothly fill the hole
//   Image image(imageIn.width, imageIn.height, 1, imageIn.channels);
//   image.CopyData(Inpaint::apply(imageIn, mask));
// 
//   image.WritePNG("image_smooth_filled.png");
// 
//   Image out(image.width, image.height, 1, image.channels);
//   out.CopyData(image);
  //out.Zero();

  Image image(imageIn.width, imageIn.height, 1, imageIn.channels);
  image.CopyData(imageIn);

  Image out(image.width, image.height, 1, image.channels);
  out.CopyData(image);
  
  Image patchMatch;

  Image initialization = Image(image.width, image.height, 1, image.channels);

  for(int i = 0; i < numIter; ++i)
  {
    std::cout << "Heal Iteration " << i << std::endl;

    std::stringstream ssMask;
    ssMask << "Mask_" << i << ".png";
    mask.WriteMask(ssMask.str());

    if(i == 0)
    {
      PatchMatch::Random(initialization, image, mask, patchDiameter);
    }
    else
    {
      initialization.CopyData(patchMatch);
    }

    //initialization.WriteMeta("initPatchMask.mha");
    
    patchMatch = PatchMatch::apply(image, image, mask,
                                   numIterPM, patchDiameter, initialization);
    std::stringstream ssPatchMatch;
    ssPatchMatch << "Heal_PatchMatch_" << i << ".mha";
    patchMatch.WriteMeta(ssPatchMatch.str());
    std::cout << "Finished PatchMatch." << std::endl;

    // The contribution of each pixel q to the error term (d_cohere) = 1/N_T \sum_{i=1}^m (S(p_i) - T(q))^2
    // To find the best color T(q) (iterative update rule), differentiate with respect to T(q),
    // set to 0, and solve for T(q):
    // T(q) = \frac{1}{m} \sum_{i=1}^m S(p_i)

    // Loop over the whole image (patch centers)
    unsigned int numberOfPixelsFilled = 0;
    for(int y = 0; y < image.height; ++y)
    {
      for(int x = 0; x < image.width; ++x)
      {
        if(mask(x,y)[0] == 0) // We have come across a pixel to be filled
        {
          out.SetAllComponents(x, y, 0.0f);

          numberOfPixelsFilled++;

          unsigned int numberOfContributors = 0;

          // Iterate over the patch centered on this pixel
          for (int dy = -patchDiameter/2; dy <= patchDiameter/2; ++dy)
          {
            if (y+dy < 0)// skip this row
            {
              continue;
            }
            if (y+dy >= out.height) // quit when we get past the last row
            {
              break;
            }

            for(int dx = -patchDiameter/2; dx <= patchDiameter/2; ++dx)
            {
              if (x+dx < 0) // skip this pixel
              {
                continue;
              }
              else if(x+dx >= out.width) // quit when we get to the end of the row
              {
                break;
              }
              else // Use a pixel from this patch in the update
              {
                // (matchX, matchY) is the center of the best matching patch to the patch centered at (x+dx, y+dy)
                int matchX = (int)patchMatch(x+dx,y+dy)[0];
                int matchY = (int)patchMatch(x+dx,y+dy)[1];

                numberOfContributors++;
                for(int c = 0; c < image.channels; ++c)
                {
                  out(x, y)[c] += image(matchX-dx, matchY-dy)[c];
                }
              }
            } // end loop over row
          } // end loop over patch
          //std::cout << "numberOfContributors: " << numberOfContributors << std::endl;
          //float weight = 1.0f/static_cast<float>(numberOfContributors);
          //std::cout << "weight: " << weight << std::endl;
          for(int c = 0; c < image.channels; ++c)
          {
            out(x,y)[c] /= static_cast<float>(numberOfContributors);
          }
        } // end if (!targetMask || targMaskPtr[0] > 0)
      }
    } // end loop over image

    std::stringstream ssMeta;
    ssMeta << "Iteration_" << i << ".mha";
    out.WriteMeta(ssMeta.str());

    std::stringstream ssPNG;
    ssPNG << "Iteration_" << Helpers::ZeroPad(i, 2) << ".png";
    out.WritePNG(ssPNG.str());

    // reset for the next iteration
    //image = out;
    //image = out.copy();
    image.CopyData(out);

    std::cout << "numberOfPixelsFilled: " << numberOfPixelsFilled << std::endl;
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
  //int numIter = 5;
  //int numIter = 10;
  int numIter = 50;
  int numIterPM = 5;
  int patchDiameter = 15;
  
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

  std::cout << "Running Heal with numIter " << numIter << " and numIterPM " << numIterPM << std::endl;
  apply(image, mask, numIter, numIterPM, patchDiameter);
}

#include "footer.h"
