#include "main.h"
#include "Image.h"

#include "itkVectorImage.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileWriter.h"

#include "header.h"

// The rest of the Image class is inlined.
// Inlining the destructor makes the compiler unhappy, so it goes here instead

// If you think there is a bug here, there is almost certainly
// actually a bug somewhere else which is corrupting the memory table
// or the image header. It will often crash here as a result because
// this is where things get freed.

Image::~Image() {    
    //printf("In image destructor\n"); fflush(stdout);

    if (!refCount) {
        //printf("Deleting NULL image\n"); fflush(stdout);
        return; // the image was a dummy
    }
    //printf("Decremementing refcount\n"); fflush(stdout);
    refCount[0]--;
    if (*refCount <= 0) {
        //printf("Deleting image\n"); fflush(stdout);
        //debug();
        delete refCount;
        //printf("refCount deleted\n"); fflush(stdout);
        //debug();        
        delete[] memory;
        //printf("data deleted\n"); fflush(stdout);
        //debug();
    }

    //printf("Leaving image desctructor\n"); fflush(stdout);
}

void Window::Write(const std::string& filename)
{
  typedef itk::VectorImage<float, 2> ImageType;
  ImageType::Pointer itkimage = ImageType::New();
  itk::Index<2> itkcorner = {{0,0}};
  itk::Size<2> itksize = {{this->width, this->height}};
  itk::ImageRegion<2> itkregion(itkcorner, itksize);
  itkimage->SetRegions(itkregion);
  itkimage->SetNumberOfComponentsPerPixel(this->channels);
  itkimage->Allocate();

  itk::ImageRegionIterator<ImageType> imageIterator(itkimage, itkregion);

  while(!imageIterator.IsAtEnd())
    {
    ImageType::PixelType pixel;
    pixel.SetSize(this->channels);
    for(unsigned int channel = 0; channel < this->channels; ++channel)
      {
      pixel[channel] = this->operator()(imageIterator.GetIndex()[0], imageIterator.GetIndex()[1])[channel];
      }
    imageIterator.Set(pixel);

    ++imageIterator;
    }

  typedef  itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(filename);
  writer->SetInput(itkimage);
  writer->Update();
}


#include "footer.h"
