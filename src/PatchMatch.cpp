
// CS448F final project
// Implementation of PatchMatch algorithm and its applications
// Sung Hee Park (shpark7@stanford.edu)

#include "itkVectorImage.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileWriter.h"

#include "main.h"
#include "File.h"
#include "Geometry.h"
#include "PatchMatch.h"
#include "Arithmetic.h"
#include "Calculus.h"
#include "Statistics.h"
#include "Filter.h"
#include "Paint.h"
#include "header.h"
// PATCHMATCH =============================================================//

void PatchMatch::help() {

    printf("-patchmatch computes approximate nearest neighbor field from the top\n"
           "image on the stack to the second image on the stack, using the\n"
           "algorithm from the PatchMatch SIGGRAPH 2009 paper. This operation\n"
           "requires two input images which may have multiple frames.\n"
           "It returns an image with four channels. First three channels \n"
           "correspond to x, y, t coordinate of closest patch and \n"
           "fourth channels contains the sum of squared differences \n"
           "between patches. \n"
           "\n"
           " arguments [numIter] [patchSize]\n"
           "  - numIter : number of iterations performed. (default: 5)\n"
           "  - patchSize : size of patch. (default: 7, 7x7 square patch)\n"
           " You can omit some arguments from right to use default values.\n"
           "\n"
           "Usage: ImageStack -load target.jpg -load source.jpg -patchmatch -save match.tmp\n\n");

    // NOTE: This is not the notation used in the original PatchMatch paper - the nearest neighbor field is stored as offsets, where in
    // this code it is stored as absolute locations of the best matching patches.
}

void PatchMatch::parse(vector<string> args) {

    int numIter = 5, patchSize = 7;

    assert(args.size() <= 2, "-patchmatch takes two or fewer arguments\n");
    if (args.size() == 2) {
        numIter = readInt(args[0]);
        patchSize = (int) readInt(args[1]);
    } else if (args.size() == 1) {
        numIter = readInt(args[0]);
    }

    Image result;                     

    //result = apply(stack(0), stack(1), numIter, patchSize); // no mask
    std::cout << "stack(0) has " << stack(0).channels << std::endl;
    std::cout << "stack(1) has " << stack(1).channels << std::endl;
    std::cout << "stack(2) has " << stack(2).channels << std::endl;
    result = apply(stack(1), stack(2), stack(0), numIter, patchSize); // with mask as third image to be loaded (-load)

    push(result);
}

Image PatchMatch::apply(Window source, Window target, int iterations, int patchSize) {
    return apply(source, target, Window(), iterations, patchSize);
}

Image PatchMatch::apply(Window source, Window target, Window mask, int iterations, int patchSize) {

    if (mask) {
        assert(target.width == mask.width &&
               target.height == mask.height &&
               target.frames == mask.frames, 
               "Mask must have the same dimensions as the target\n");
        std::cout << "mask has " << mask.channels << " channels." << std::endl;
        assert(mask.channels == 1,
               "Mask must have a single channel\n");
    }
    assert(iterations > 0, "Iterations must be a strictly positive integer\n");
    assert(patchSize >= 3 && (patchSize & 1), "Patch size must be at least 3 and odd\n");

    // convert patch diameter to patch radius
    patchSize /= 2;

    // For each source pixel, output a 3-vector to the best match in
    // the target, with an error as the last channel.
    Image out(source.width, source.height, source.frames, 4);
    
    // Iterate over source frames, finding a match in the target where
    // the mask is high
    
    float *outPtr = out(0, 0, 0);
    //for (int t = 0; t < source.frames; t++) {
    int t = 0; // We are only dealing with single frame images (non-video)
    
    // INITIALIZATION - uniform random assignment
    for(int y = 0; y < source.height; y++) {
        for(int x = 0; x < source.width; x++) {
            int dx = randomInt(patchSize, target.width-patchSize-1);
            int dy = randomInt(patchSize, target.height-patchSize-1);
            int dt = randomInt(0, target.frames-1);
            *outPtr++ = dx;
            *outPtr++ = dy;
            *outPtr++ = dt;
            *outPtr++ = distance(source, target, mask,
                                  t, x, y,
                                  dt, dx, dy,
                                  patchSize, HUGE_VAL);
        }
    }
    

    bool forwardSearch = true;

    for (int i = 0; i < iterations; i++)
    {
      //printf("Iteration %d\n", i);

      // PROPAGATION
      if (forwardSearch)
      {
          // Forward propagation - compare left, center and up
          //for (int t = 0; t < source.frames; t++) {
              for(int y = 1; y < source.height; y++)
              {
                  outPtr = out(1, y, t);
                  float *leftPtr = out(0, y, t);
                  float *upPtr = out(1, y-1, t);
                  for(int x = 1; x < source.width; x++)
                  {

                      if (outPtr[3] > 0)
                      {
                          float distLeft = distance(source, target, mask,
                                                    t, x, y,
                                                    leftPtr[2], leftPtr[0]+1, leftPtr[1],
                                                    patchSize, outPtr[3]);

                          if (distLeft < outPtr[3])
                          {
                              outPtr[0] = leftPtr[0]+1;
                              outPtr[1] = leftPtr[1];
                              outPtr[2] = leftPtr[2];
                              outPtr[3] = distLeft;
                          }

                          float distUp = distance(source, target, mask,
                                                  t, x, y,
                                                  upPtr[2], upPtr[0], upPtr[1]+1,
                                                  patchSize, outPtr[3]);

                          if (distUp < outPtr[3])
                          {
                              outPtr[0] = upPtr[0];
                              outPtr[1] = upPtr[1]+1;
                              outPtr[2] = upPtr[2];
                              outPtr[3] = distUp;
                          }
                      }

                      outPtr += 4;
                      leftPtr += 4;
                      upPtr += 4;

                  }
              }
          

        }
        else
        {                
          // Backward propagation - compare right, center and down

              for(int y = source.height-2; y >= 0; y--)
              {
                  outPtr = out(source.width-2, y, t);
                  float *rightPtr = out(source.width-1, y, t);
                  float *downPtr = out(source.width-2, y+1, t);
                  for(int x = source.width-2; x >= 0; x--)
                  {
                      if (outPtr[3] > 0)
                      {
                          float distRight = distance(source, target, mask,
                                                      t, x, y,
                                                      rightPtr[2], rightPtr[0]-1, rightPtr[1],
                                                      patchSize, outPtr[3]);

                          if (distRight < outPtr[3])
                          {
                              outPtr[0] = rightPtr[0]-1;
                              outPtr[1] = rightPtr[1];
                              outPtr[2] = rightPtr[2];
                              outPtr[3] = distRight;
                          }

                          float distDown = distance(source, target, mask,
                                                    t, x, y,
                                                    downPtr[2], downPtr[0], downPtr[1]-1,
                                                    patchSize, outPtr[3]);

                          if (distDown < outPtr[3])
                          {
                              outPtr[0] = downPtr[0];
                              outPtr[1] = downPtr[1]-1;
                              outPtr[2] = downPtr[2];
                              outPtr[3] = distDown;
                          }
                      }

                      outPtr -= 4;
                      rightPtr -= 4;
                      downPtr -= 4;
                  }
              }
          
        }            

        forwardSearch = !forwardSearch;

        // RANDOM SEARCH
        float *outPtr = out(0, 0, 0);

        for(int y = 0; y < source.height; y++)
        {
          for(int x = 0; x < source.width; x++)
          {

            if (outPtr[3] > 0)
            {

              int radius = target.width > target.height ? target.width : target.height;

              // search an exponentially smaller window each iteration
              while (radius > 8)
              {
                // Search around current offset vector (distance-weighted)

                // clamp the search window to the image
                int minX = (int)outPtr[0] - radius;
                int maxX = (int)outPtr[0] + radius + 1;
                int minY = (int)outPtr[1] - radius;
                int maxY = (int)outPtr[1] + radius + 1;
                if (minX < 0) minX = 0;
                if (maxX > target.width) maxX = target.width;
                if (minY < 0) minY = 0;
                if (maxY > target.height) maxY = target.height;

                int randX = rand() % (maxX - minX) + minX;
                int randY = rand() % (maxY - minY) + minY;
                int randT = rand() % target.frames;
                float dist = distance(source, target, mask,
                                      t, x, y,
                                      randT, randX, randY,
                                      patchSize, outPtr[3]);
                if (dist < outPtr[3])
                {
                  outPtr[0] = randX;
                  outPtr[1] = randY;
                  outPtr[2] = randT;
                  outPtr[3] = dist;
                }

                radius >>= 1;

              }
            }
            outPtr += 4;
          }
      }

    }

    typedef itk::VectorImage<float, 2> ImageType;
    ImageType::Pointer itkimage = ImageType::New();
    itk::Index<2> itkcorner = {{0,0}};
    itk::Size<2> itksize = {{out.width,out.height}};
    itk::ImageRegion<2> itkregion(itkcorner, itksize);
    itkimage->SetRegions(itkregion);
    itkimage->SetNumberOfComponentsPerPixel(out.channels);
    itkimage->Allocate();

    itk::ImageRegionIterator<ImageType> imageIterator(itkimage, itkregion);

    while(!imageIterator.IsAtEnd())
      {
      ImageType::PixelType pixel;
      pixel.SetSize(out.channels);
      for(unsigned int channel = 0; channel < out.channels; ++channel)
        {
        pixel[channel] = out(imageIterator.GetIndex()[0], imageIterator.GetIndex()[1])[channel];
        }
      imageIterator.Set(pixel);

      ++imageIterator;
      }

    typedef  itk::ImageFileWriter< ImageType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName("nnfield.mha");
    writer->SetInput(itkimage);
    writer->Update();

    return out;
}

float PatchMatch::distance(Window source, Window target, Window mask,
                           int st, int sx, int sy, 
                           int tt, int tx, int ty,
                           int patchSize, float prevDist)
{

    // Do not use patches on boundaries
    if (tx < patchSize || tx >= target.width-patchSize || 
        ty < patchSize || ty >= target.height-patchSize)
    {
        return HUGE_VAL;
    }

    // Compute distance between patches
    // Average L2 distance in RGB space
    float dist = 0;
    float weight = 0;

    float threshold = prevDist*target.channels*(2*patchSize+1)*(2*patchSize+1);

    int x1 = max(-patchSize, -sx, -tx);
    int x2 = min(patchSize, -sx+source.width-1, -tx+target.width-1);
    int y1 = max(-patchSize, -sy, -ty);
    int y2 = min(patchSize, -sy+source.height-1, -ty+target.height-1);
    
    /*
    int x1 = -patchSize, x2 = patchSize;
    int y1 = -patchSize, y2 = patchSize;
    */


    for(int y = y1; y <= y2; y++)
    {

        float *pSource = source(sx+x1, sy+y, st);
        float *pTarget = target(tx+x1, ty+y, tt);
        float *pMask = NULL;
        if (mask)
        {
          pMask = mask(tx+x1, ty+y, tt);
        }

        for (int i = 0; i <= x2-x1; i++)
        {
            float d = 0;
            float w = mask ? pMask[0] : 1; // If there is a mask, set the weight to the mask value. If not, set all weights to 1.
            assert(w >= 0, "Negative w %f\n", w);
            for (int j = 0; j < target.channels; j++)
            {
              d += w*(*pSource - *pTarget)*(*pSource - *pTarget);
              weight += w;
              pSource++; pTarget++;
            }

            if (mask)
            {
              pMask++;
            }

            dist += d;

            // Early termination
            if (dist > threshold)
            {
              return HUGE_VAL;
            }
        }
    }

    assert(dist >= 0, "negative dist\n");
    assert(weight >= 0, "negative weight\n");

    if (!weight) return HUGE_VAL;

    return dist / weight;
}

#include "footer.h"
