/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

#ifndef DDS_H
#define DDS_H

#include <vector>

#include "image.h"

enum DDSFormat {
    DDSF_R5G6B5,
    DDSF_R8G8B8,
    DDSF_A8R8G8B8,
    DDSF_A2R10G10B10,
    DDSF_A1R5G5B5,
    DDSF_R8,
    DDSF_R16,
    DDSF_G16R16,
    DDSF_A8R8,
    DDSF_A4R4,
    DDSF_A16R16,
    DDSF_R3G3B2,
    DDSF_A4R4G4B4,
    DDSF_BC1,  // fourcc: DXT1
    DDSF_BC2,  // fourcc: DXT2 or DXT3
    DDSF_BC3,  // fourcc: DXT4 or DXT5
    DDSF_BC4,  // fourcc: ATI1
    DDSF_BC5,  // fourcc: ATI2
    // Some other weird formats that cropped up in Ogre example dds files, undocumented
    // in MSDN but I found some other code that handles them:
    // https://sourceforge.net/p/tgtl/code/HEAD/tree/trunk/src/Filters/DDSFilter.hpp#l422
    // There are probably more out there, as this doesn't look like a complete list.
    // Actually, it looks like this is the list:
    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb172558(v=vs.85).aspx
    DDSF_R16F, // fourcc: 0x6f
    DDSF_G16R16F, // fourcc: 0x70
    DDSF_R16G16B16A16F, // fourcc: 0x71
    DDSF_R32F, // fourcc: 0x72
    DDSF_G32R32F, // fourcc: 0x73
    DDSF_R32G32B32A32F, // fourcc: 0x74
};

/** Bitwise OR of your chosen quality, and optionally enable perceptual colour error and/or alpha weighting. */
enum SquishFlags {
    SQUISH_QUALITY_HIGHEST = 1, // iterative cluster fit: slow
    SQUISH_QUALITY_HIGH = 2, // cluster fit (recommended)
    SQUISH_QUALITY_LOW = 3, // range fit

    SQUISH_METRIC_PERCEPTUAL = 4, //  for colour error (recommended for diffuse textures)

    SQUISH_WEIGHT_COLOUR_BY_ALPHA = 8, // Weight the colour by alpha during cluster fit
};

typedef std::vector<ImageBase*> ImageBases;

struct DDSCube {
    ImageBases X, x, Y, y, Z, z;
};

DDSFormat format_from_string (const std::string &str);
std::string format_to_string (DDSFormat fmt);

enum DDSFileType {
    DDS_SIMPLE,
    DDS_CUBE,
    DDS_VOLUME,
};

struct DDSFile {
    DDSFileType kind;
    ImageBases simple;
    DDSCube cube;
    // The top-level vector contains the mip levels, inside each of which is all the layers for that level.
    std::vector<ImageBases> volume;
};

void dds_save (const std::string &filename, DDSFormat format, const DDSFile &content, int flags);
DDSFile dds_open (const std::string &filename);

#endif
