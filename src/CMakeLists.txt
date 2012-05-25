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
Prediction.cpp # has Inpaint()
Stack.cpp
Statistics.cpp
BidirectionalSimilarity.cpp

)

message("FFTW3_LIBRARIES ${FFTW3_LIBRARIES}")
TARGET_LINK_LIBRARIES(ImageStack png ${FFTW3_LIBRARIES} ${ITK_LIBRARIES})
