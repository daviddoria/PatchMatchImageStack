#include "main.h"
#include "Operation.h"

// Include your operation's source file or header here.
// It should must define a subclass of Operation (in Operation.h).

// You might want to look in Arithmetic.cpp/Arithmetic.h for some examples.

// Also check main.h for functions available to parse input, handle errors, etc.

#include "Control.h"
#include "Statistics.h"
#include "Arithmetic.h"
#include "Stack.h"
#include "Geometry.h"
#include "Convolve.h"
#include "Color.h"
#include "Complex.h"
#include "Paint.h"
#include "File.h"
#include "Calculus.h"
#include "DFT.h"
#include "Prediction.h"
#include "Filter.h"
#include "PatchMatch.h"
#include "BidirectionalSimilarity.h"
#include "LAHBPCG.h"

#include "header.h"

// add your operation to the operations table here
void loadOperations() {
    operationMap["-help"] = new Help();

    // program control
    operationMap["-loop"] = new Loop();
    operationMap["-pause"] = new Pause();
    operationMap["-time"] = new Time();

    // file IO
    operationMap["-load"] = new Load();
    operationMap["-save"] = new Save();

    // stack ops
    operationMap["-pop"] = new Pop();
    operationMap["-push"] = new Push();
    operationMap["-pull"] = new Pull();
    operationMap["-dup"] = new Dup();

    // geometry
    operationMap["-resample"] = new Resample();
    operationMap["-crop"] = new Crop();
    operationMap["-flip"] = new Flip();
    operationMap["-adjoin"] = new Adjoin();
    operationMap["-transpose"] = new Transpose();
    operationMap["-translate"] = new Translate();
    operationMap["-paste"] = new Paste();
    operationMap["-downsample"] = new Downsample();
    operationMap["-upsample"] = new Upsample();
    operationMap["-rotate"] = new Rotate();
    operationMap["-affinewarp"] = new AffineWarp();
    operationMap["-tile"] = new Tile();
    operationMap["-subsample"] = new Subsample();
    operationMap["-warp"] = new Warp();
    operationMap["-interleave"] = new Interleave();
    operationMap["-deinterleave"] = new Deinterleave();
    operationMap["-tileframes"] = new TileFrames();
    operationMap["-frametiles"] = new FrameTiles();
    operationMap["-reshape"] = new Reshape();

    // convolutions
    operationMap["-convolve"] = new Convolve();
    operationMap["-deconvolve"] = new Deconvolve();

    // calculus
    operationMap["-gradient"] = new Gradient();
    operationMap["-integrate"] = new Integrate();
    operationMap["-gradmag"] = new GradMag();
    operationMap["-poisson"] = new Poisson();

    #ifndef NO_FFTW
    // discrete fourier transforms
    operationMap["-dct"] = new DCT();
    operationMap["-fft"] = new FFT();
    operationMap["-ifft"] = new IFFT();
    operationMap["-fftconvolve"] = new FFTConvolve();
    operationMap["-fftdeconvolve"] = new FFTDeconvolve();
    operationMap["-fftpoisson"] = new FFTPoisson();
    #endif

    // painting stuff
    operationMap["-eval"] = new Eval();
    operationMap["-evalchannels"] = new EvalChannels();
    operationMap["-plot"] = new Plot();
    operationMap["-composite"] = new Composite();

    // prediction stuff
    operationMap["-inpaint"] = new Inpaint();

    // PatchMatch stuff
    operationMap["-patchmatch"] = new PatchMatch();
    operationMap["-bidirectionalsimilarity"] = new BidirectionalSimilarity();
    operationMap["-heal"] = new Heal();
}


void unloadOperations() {
    OperationMapIterator i;
    for (i = operationMap.begin(); i != operationMap.end(); ++i) {
        delete i->second;
    }
}


void Help::help() {
    pprintf("ImageStack is a stack language for manipulating images. It"
            " is appropriate for use on the command line and in scripts."
            " Internally, all data is stored as 32 bit floating point, so"
            " ImageStack is good for high dynamic range data. ImageStack"
            " is also useful for low dynamic range data, which it treats as"
            " values between 0 and 1.\n\n"
            "-help provides help on a given operation.\n"
            "Usage: ImageStack -help scale\n\n"
            "Operations available are:\n");

    OperationMapIterator i;

    for (i = operationMap.begin(); i != operationMap.end(); ++i) {
        printf("%s", i->first.c_str());
        printf(" ");
    }
    printf("\n");
}


void Help::parse(vector<string> args) {
    if (args.size() < 1) {
        help();
    } else {
        string opname = '-' + args[0];
        OperationMapIterator op = operationMap.find(opname);
        if (op == operationMap.end())
            printf("No such operation \"%s\"\n", args[0].c_str());
        else {
            printf("\n");
            op->second->help();
        }
    }
}

#include "footer.h"
