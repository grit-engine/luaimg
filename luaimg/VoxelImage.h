/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

class VoxelImage;

#ifndef VOXEL_IMAGE_H
#define VOXEL_IMAGE_H

#include <volpack.h>
#include "Image.h"

class VoxelImage {

    vpContext *vpc;

    const int fieldRgb16 = 0;
    const int fieldScalar = 1;
    const int fieldGradient = 2;

    struct Voxel {
        unsigned short rgb16;
        unsigned char scalar;
        unsigned char gradient;
    };

    Voxel *voxelData;

    float colourTable[3*(VP_NORM_MAX+1)];
    float densityRamp[256];
    

    public:

    const imglen_t width, height, depth;

    VoxelImage (float *raw_data, chan_t channels, imglen_t width, imglen_t height, imglen_t depth, bool axes);

    ~VoxelImage (void);

    void render (Image<3> *image, float euler_x, float euler_y, float euler_z);

};

static inline std::ostream &operator<<(std::ostream &o, VoxelImage &img)
{
    o << "Image ("<<img.width<<","<<img.height<<","<<img.depth<<") [0x"<<&img<<"]";
    return o;
}

#endif
