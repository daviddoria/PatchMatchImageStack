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

void PatchMatch::parse(vector<string> args)
{

    int numIter = 5, patchSize = 7;

    assert(args.size() <= 2, "-patchmatch takes two or fewer arguments\n");
    if (args.size() == 2)
    {
        numIter = readInt(args[0]);
        patchSize = (int) readInt(args[1]);
    }
    else if (args.size() == 1)
    {
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

Image PatchMatch::apply(Window source, Window target, int iterations, int patchSize)
{
    return apply(source, target, Window(), iterations, patchSize);
}

Image PatchMatch::apply(Window source, Window target, Window mask, int iterations, int patchSize)
{

    if (mask)
    {
      assert(target.width == mask.width &&
              target.height == mask.height &&
              target.frames == mask.frames,
              "Mask must have the same dimensions as the target\n");
      std::cout << "mask has " << mask.channels << " channels." << std::endl;
      assert(mask.channels == 1, "Mask must have a single channel\n");
      assert(target.frames == 1, "Target must have a single frame (non-video)\n");
      assert(source.frames == 1, "Source must have a single frame (non-video)\n");
    }
    assert(iterations > 0, "Iterations must be a strictly positive integer\n");
    assert(patchSize >= 3 && (patchSize & 1), "Patch size must be at least 3 and odd\n");

    // convert patch diameter to patch radius
    patchSize /= 2;

    // For each source pixel, output a 2-vector (x coord, y coord, error)to the best match in
    // the target.
    Image out(source.width, source.height, 1, 3); // 1 frame, 3 channels (x,y,error)

    unsigned int errorChannel = out.channels - 1; // The last channel (e.g. channel 2 (0-indexed) in a 3-channel image) is the error
    // Iterate over source frames, finding a match in the target where
    // the mask is high

    float *outPtr = out(0, 0);

    // INITIALIZATION - uniform random assignment
    for(int y = 0; y < source.height; y++)
    {
      for(int x = 0; x < source.width; x++)
      {
        int dx = randomInt(patchSize, target.width-patchSize-1);
        int dy = randomInt(patchSize, target.height-patchSize-1);
        *outPtr++ = dx;
        *outPtr++ = dy;
        *outPtr++ = distance(source, target, mask,
                              x, y,
                              dx, dy,
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
        for(int y = 1; y < source.height; ++y)
        {
          outPtr = out(1, y);
          float *leftPtr = out(0, y);
          float *upPtr = out(1, y-1);
          for(int x = 1; x < source.width; ++x)
          {
            if (outPtr[errorChannel] > 0)
            {
              float distLeft = distance(source, target, mask,
                                        x, y,
                                        leftPtr[0]+1, leftPtr[1],
                                        patchSize, outPtr[errorChannel]);

              if (distLeft < outPtr[errorChannel])
              {
                  outPtr[0] = leftPtr[0]+1;
                  outPtr[1] = leftPtr[1];
                  outPtr[errorChannel] = distLeft;
              }

              float distUp = distance(source, target, mask,
                                      x, y,
                                      upPtr[0], upPtr[1]+1,
                                      patchSize, outPtr[errorChannel]);

              if (distUp < outPtr[errorChannel])
              {
                  outPtr[0] = upPtr[0];
                  outPtr[1] = upPtr[1]+1;
                  outPtr[errorChannel] = distUp;
              }
            } // end if (outPtr[3] > 0)

            outPtr += out.channels;
            leftPtr += out.channels;
            upPtr += out.channels;

          } // end for(int x = 1; x < source.width; x++)
        } // end for(int y = 1; y < source.height; y++)
      } // end if (forwardSearch)
      else
      {
        // Backward propagation - compare right, center and down

        for(int y = source.height-2; y >= 0; --y)
        {
          outPtr = out(source.width-2, y);
          float *rightPtr = out(source.width-1, y);
          float *downPtr = out(source.width-2, y+1);
          for(int x = source.width-2; x >= 0; --x)
          {
            if (outPtr[errorChannel] > 0)
            {
              float distRight = distance(source, target, mask,
                                          x, y,
                                          rightPtr[0]-1, rightPtr[1],
                                          patchSize, outPtr[errorChannel]);

              if (distRight < outPtr[errorChannel])
              {
                  outPtr[0] = rightPtr[0]-1;
                  outPtr[1] = rightPtr[1];
                  outPtr[errorChannel] = distRight;
              }

              float distDown = distance(source, target, mask,
                                        x, y,
                                        downPtr[0], downPtr[1]-1,
                                        patchSize, outPtr[errorChannel]);

              if (distDown < outPtr[errorChannel])
              {
                  outPtr[0] = downPtr[0];
                  outPtr[1] = downPtr[1]-1;
                  outPtr[errorChannel] = distDown;
              }
            } // end if (outPtr[3] > 0)

            outPtr -= out.channels;
            rightPtr -= out.channels;
            downPtr -= out.channels;
          } // end for(int x = source.width-2; x >= 0; x--)
        } // end for(int y = source.height-2; y >= 0; y--)
      } // end else

      forwardSearch = !forwardSearch;

      // RANDOM SEARCH
      float *outPtr = out(0, 0);

      for(int y = 0; y < source.height; ++y)
      {
        for(int x = 0; x < source.width; ++x)
        {

          if (outPtr[2] > 0)
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
              float dist = distance(source, target, mask,
                                    x, y,
                                    randX, randY,
                                    patchSize, outPtr[3]);
              if (dist < outPtr[2])
              {
                outPtr[0] = randX;
                outPtr[1] = randY;
                outPtr[2] = dist;
              }

              radius >>= 1; // Divide the radius by 2

            } // end while (radius > 8)
          } // end if (outPtr[3] > 0)
          outPtr += out.channels;
        } // end for x
      } // end for y
    } // end for (int i = 0; i < iterations; i++)

    // Zero the boundary
    for(int y = 0; y < source.height; ++y)
    {
      for(int x = 0; x < source.width; ++x)
      {
        if (x < patchSize || x >= target.width-patchSize ||
            y < patchSize || y >= target.height-patchSize)
        {
          out(x,y)[0] = 0.0f;
          out(x,y)[1] = 0.0f;
          out(x,y)[2] = 0.0f;
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

    typedef  itk::ImageFileWriter<ImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName("nnfield.mha");
    writer->SetInput(itkimage);
    writer->Update();

    return out;
}

float PatchMatch::distance(Window source, Window target, Window mask,
                           int sx, int sy,
                           int tx, int ty,
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

    // Define the patch region that is inside of the image (defined by two corners, (x1, y1) and (x2, y2) )
    int x1 = max(-patchSize, -sx, -tx);
    int x2 = min(patchSize, -sx+source.width-1, -tx+target.width-1);
    int y1 = max(-patchSize, -sy, -ty);
    int y2 = min(patchSize, -sy+source.height-1, -ty+target.height-1);

    for(int y = y1; y <= y2; y++)
    {
      float *pSource = source(sx+x1, sy+y);
      float *pTarget = target(tx+x1, ty+y);
      float *pMask = NULL;
      if (mask)
      {
        pMask = mask(tx+x1, ty+y);
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
