
/***********************************************************************************
    Created:	17:9:2002
    FileName: 	hdrloader.h
    Author:		Igor Kravtchenko

    Info:		Load HDR image and convert to a set of float32 RGB triplet.
************************************************************************************/

#ifndef HDRLOADER_H_INCLUDED
#define HDRLOADER_H_INCLUDED

#include <QDebug>

typedef unsigned char RGBE[4];
#define R			0
#define G			1
#define B			2
#define E			3

#define  MINELEN	8				// minimum scanline length for encoding
#define  MAXELEN	0x7fff			// maximum scanline length for encoding

class HDRLoaderResult {
public:
    ~HDRLoaderResult() { delete[] cols; }
    int width, height;
    // each pixel takes 3 float32, each component can be of any value...
    float *cols;
};

class HDRLoader {
public:
    static bool load(const char *fileName, HDRLoaderResult &res);
private:
    static float convertComponent(int expo, int val);
    static void workOnRGBE(RGBE *scan, int len, float *cols);
    static bool decrunch(RGBE *scanline, int len, FILE *file);
    static bool oldDecrunch(RGBE *scanline, int len, FILE *file);
};


#endif // HDRLOADER_H_INCLUDED
