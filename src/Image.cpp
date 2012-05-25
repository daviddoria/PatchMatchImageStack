#include "main.h"
#include "Image.h"

#include "itkVectorImage.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"

#include "header.h"

// The rest of the Image class is inlined.
// Inlining the destructor makes the compiler unhappy, so it goes here instead

// If you think there is a bug here, there is almost certainly
// actually a bug somewhere else which is corrupting the memory table
// or the image header. It will often crash here as a result because
// this is where things get freed.

Image::~Image()
{
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

void Window::WriteMeta(const std::string& filename)
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


void Window::WriteMask(const std::string& filename)
{
  if(this->channels != 1)
  {
    std::cerr << "Can't WriteMask with an image with " << this->channels << " channels." << std::endl;
    return;
  }

  typedef itk::Image<unsigned char, 2> ImageType;
  ImageType::Pointer itkimage = ImageType::New();
  itk::Index<2> itkcorner = {{0,0}};
  itk::Size<2> itksize = {{this->width, this->height}};
  itk::ImageRegion<2> itkregion(itkcorner, itksize);
  itkimage->SetRegions(itkregion);
  itkimage->Allocate();

  itk::ImageRegionIterator<ImageType> imageIterator(itkimage, itkregion);

  while(!imageIterator.IsAtEnd())
    {
    ImageType::PixelType pixel = 255. * this->operator()(imageIterator.GetIndex()[0],
                                                         imageIterator.GetIndex()[1])[0];
    imageIterator.Set(pixel);

    ++imageIterator;
    }

  typedef  itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(filename);
  writer->SetInput(itkimage);
  writer->Update();
}

void Window::WritePNG(const std::string& filename)
{
  if(this->channels < 3)
  {
    std::cerr << "Can't WritePNG with an image with only " << this->channels << " channels." << std::endl;
    return;
  }
  
  typedef itk::Image<itk::CovariantVector<unsigned char, 3> , 2> ImageType;
  ImageType::Pointer itkimage = ImageType::New();
  itk::Index<2> itkcorner = {{0,0}};
  itk::Size<2> itksize = {{this->width, this->height}};
  itk::ImageRegion<2> itkregion(itkcorner, itksize);
  itkimage->SetRegions(itkregion);
  itkimage->Allocate();

  itk::ImageRegionIterator<ImageType> imageIterator(itkimage, itkregion);

  while(!imageIterator.IsAtEnd())
    {
    ImageType::PixelType pixel;
    for(unsigned int channel = 0; channel < 3; ++channel)
      {
      pixel[channel] = 255. * this->operator()(imageIterator.GetIndex()[0], imageIterator.GetIndex()[1])[channel];
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

void Window::Zero()
{
  for(unsigned int x = 0; x < this->width; ++x)
  {
    for(unsigned int y = 0; y < this->height; ++y)
    {
      for(unsigned int c = 0; c < this->channels; ++c)
      {
        this->operator()(x,y)[c] = 0;
      }
    }
  }
}

#include "footer.h"
