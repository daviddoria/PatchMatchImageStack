ImageStack -load pics/dog1.png -load pics/dog1.png -patchmatch -save pics/nnfield.tmp
# If you run this with the same image for the source and target as done above, the output will have the following properties
# that indicate that the match for each patch is identically the same patch (in the same location):
# - The first channel is a gradient from left to right, indicating that the x coordinates of the best matching patches
# are increasing from left to right (with the actual pixel x-coordinates)
# - The second channel is a gradient from top to bottom, indicating that the y coordinates of the best matching patches
# are increasing from top to bottom (with the actual pixel y-coordinates)
# - The last channel is all zeros, because this is not a temporal sequence (i.e. the matches came from the same layer/image.
# - The fourth channel is all zeros, indicating that the best matches all had 0 SSD

ImageStack -load pics/dog1.png -load pics/dog1.png -load pics/dog1_patchmatch_mask.png -patchmatch -save pics/nnfield.tmp