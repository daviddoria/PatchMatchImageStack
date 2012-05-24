ADD_EXECUTABLE(ImageStack
Arithmetic.cpp
Calculus.cpp
Color.cpp
Complex.cpp
Control.cpp
Convolve.cpp
DFT.cpp
Exception.cpp
File.cpp
FilePNG.cpp
FileTMP.cpp
Filter.cpp
Geometry.cpp
HDR.cpp
Image.cpp
LAHBPCG.cpp
main.cpp
Operation.cpp
Paint.cpp
Parser.cpp
PatchMatch.cpp
Prediction.cpp # has Inpaint()
Stack.cpp
Statistics.cpp
)

TARGET_LINK_LIBRARIES(ImageStack tiff png jpeg fftw3f ${ITK_LIBRARIES}) # jpeg is required when building with ITK? Strange...
