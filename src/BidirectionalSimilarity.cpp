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
#include "header.h"

void BidirectionalSimilarity::help()
{
    pprintf("-bidirectionalsimilarity reconstructs the top image on the stack using"
            " patches from the second image on the stack, by enforcing coherence"
            " (every patch in the output must look like a patch from the input) and"
            " completeness (every patch from the input must be represented somewhere"
            " in the output). The first argument is a number between zero and one,"
            " which trades off between favoring coherence only (at zero), and"
            " completeness only (at one). It defaults to 0.5. The second arguments"
            " specifies the number of iterations that should be performed, and"
            " defaults to five. Bidirectional similarity uses patchmatch as the"
            " underlying nearest-neighbour-field algorithm, and the third argument"
            " specifies how many iterations of patchmatch should be performed each"
            " time it is run. This also defaults to five.\n"
            "\n"
            "Usage: ImageStack -load source.jpg -load target.jpg -bidirectional 0.5 -display\n");
}

void BidirectionalSimilarity::parse(vector<string> args)
{

    float alpha = 0.5;
    int numIter = 5;
    int numIterPM = 5;

    assert(args.size() <= 3, "-bidirectional takes three or fewer arguments\n");
    if (args.size() == 3) {
         alpha = readFloat(args[0]);
        numIter = readFloat(args[1]);
        numIterPM = readFloat(args[2]);
    } else if (args.size() == 2) {
        alpha = readFloat(args[0]);
        numIter = readFloat(args[1]);
    } else if (args.size() == 1) {
        alpha = readFloat(args[0]);
    }

    apply(stack(1), stack(0), Window(), Window(), alpha, numIter, numIterPM);
}


// Reconstruct the portion of the target where the mask is high, using
// the portion of the source where its mask is high. Source and target
// masks are allowed to be null windows.
void BidirectionalSimilarity::apply(Window source, Window target,
                                    Window sourceMask, Window targetMask,
                                    float alpha, int numIter, int numIterPM)
{
  // TODO: intelligently crop the input to where the mask is high +
  // patch radius on each side

  // recurse
  if (source.width > 32 && source.height > 32 && target.width > 32 && target.height > 32)
  {
    Image smallSource = Resample::apply(source, source.width/2, source.height/2, source.frames);
    Image smallTarget = Resample::apply(target, target.width/2, target.height/2, target.frames);

    Image smallSourceMask;
    Image smallTargetMask;
    if (sourceMask)
    {
      smallSourceMask = Downsample::apply(sourceMask, 2, 2, 1);
    }

    if (targetMask)
    {
      smallTargetMask = Downsample::apply(targetMask, 2, 2, 1);
    }

    apply(smallSource, smallTarget, smallSourceMask, smallTargetMask, alpha, numIter, numIterPM);

    Image newTarget = Resample::apply(smallTarget, target.width, target.height, target.frames);

    if (targetMask)
    {
      Composite::apply(target, newTarget, targetMask);
    }
    else
    {
      for (int t = 0; t < target.frames; t++)
      {
        for (int y = 0; y < target.height; y++)
        {
          float *targPtr = target(0, y, t);
          float *newTargPtr = newTarget(0, y, t);
          memcpy(targPtr, newTargPtr, sizeof(float)*target.channels*target.width);
        }
      }
    }
  }

  printf("%dx%d ", target.width, target.height); fflush(stdout);
  for(int i = 0; i < numIter; i++)
  {
    printf("."); fflush(stdout);

    int patchSize = target.width / 6;

    Image completeMatch, coherentMatch;

    // The homogeneous output for this iteration
    Image out(target.width, target.height, target.frames, target.channels+1);

    if (alpha != 0)
    {

      // COMPLETENESS TERM
      Image completeMatch = PatchMatch::apply(source, target, targetMask, numIterPM, patchSize);

      // For every patch in the source, splat it onto the
      // nearest match in the target, weighted by the source
      // mask and also by the inverse of the patch distance
      float *matchPtr = completeMatch(0, 0, 0);
      for (int t = 0; t < source.frames; t++)
      {
        for (int y = 0; y < source.height; y++)
        {
          float *srcMaskPtr = sourceMask(0, y, t);
          for (int x = 0; x < source.width; x++)
          {

            if (!sourceMask || srcMaskPtr[0] > 0)
            {

                int dstX = (int)matchPtr[0];
                int dstY = (int)matchPtr[1];
                int dstT = (int)matchPtr[2];
                float weight = 1.0f/(matchPtr[3]+1);

                if (sourceMask)
                {
                  weight *= srcMaskPtr[0];
                }

              for (int dy = -patchSize/2; dy <= patchSize/2; dy++)
              {
                if (y+dy < 0)
                {
                  continue;
                }
                if (y+dy >= source.height)
                {
                  break;
                }
                float *sourcePtr = source(x-patchSize/2, y+dy, t);
                float *outPtr = out(dstX-patchSize/2, dstY+dy, dstT);
                for (int dx = -patchSize/2; dx <= patchSize/2; dx++)
                {
                  if (x+dx < 0)
                  {
                    outPtr += out.channels;
                    sourcePtr += source.channels;
                  }
                  else if (x+dx >= source.width)
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
            }

            srcMaskPtr++;
            matchPtr += completeMatch.channels;
          }
        }
      }
    }

      if (alpha != 1)
      {
        // COHERENCE TERM
        Image coherentMatch = PatchMatch::apply(target, source, sourceMask,
                                                numIterPM, patchSize);
        // For every patch in the target, pull from the nearest match in the source
        float *matchPtr = coherentMatch(0, 0, 0);
        for (int t = 0; t < target.frames; t++)
        {
          for (int y = 0; y < target.height; y++)
          {
            float *targMaskPtr = targetMask(0, y, t);
            for (int x = 0; x < target.width; x++)
            {
              if (!targetMask || targMaskPtr[0] > 0)
              {
                int dstX = (int)matchPtr[0];
                int dstY = (int)matchPtr[1];
                int dstT = (int)matchPtr[2];
                float weight = 1.0f/(matchPtr[3]+1);

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
                  float *sourcePtr = source(dstX-patchSize/2, dstY+dy, dstT);
                  float *outPtr = out(x-patchSize/2, y+dy, t);
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
              }

                targMaskPtr++;
                matchPtr += coherentMatch.channels;
            }
          }
        }
    }

    // rewrite the target using the homogeneous output
    float *outPtr = out(0, 0, 0);
    float *targMaskPtr = targetMask(0, 0, 0);
    for (int t = 0; t < out.frames; t++)
    {
      for (int y = 0; y < out.height; y++)
      {
        float *targetPtr = target(0, y, t);
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
        }
      }
    }

    //Display::apply(target);

  }
  printf("\n");
}

void Heal::help() {
    printf("-heal takes an image and a mask, and reconstructs the portion of"
           " the image where the mask is high using patches from the rest of the"
           " image. It uses the patchmatch algorithm for acceleration. The"
           " arguments include the number of iterations to run per scale, and the"
           " number of iterations of patchmatch to run. Both default to five.\n"
           "\n"
           "Usage: ImageStack -load mask.png -load image.jpg -heal -display\n");
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

    BidirectionalSimilarity::apply(image, image, inverseMask, mask, 0, numIter, numIterPM);
}
#include "footer.h"
