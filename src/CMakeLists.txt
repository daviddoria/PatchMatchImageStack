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
Image.cpp
LAHBPCG.cpp
main.cpp
Operation.cpp
Paint.cpp
Parser.cpp
PatchMatch.cpp
Inpaint.cpp
Stack.cpp
Statistics.cpp
BidirectionalSimilarity.cpp
)

TARGET_LINK_LIBRARIES(ImageStack png ${FFTW3_LIBRARIES} ${ITK_LIBRARIES})
