/* Copyright (c) David Cunningham and the Grit Game Engine project 2013
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

#include "Image.h"

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
    DDSF_DXT1,
    DDSF_DXT3,
    DDSF_DXT5,
};

/** Bitwise OR of your chosen quality, and optionally enable perceptual colour error and/or alpha weighting. */
enum DXTFlags {
    DXT_QUALITY_HIGHEST = 1, // iterative cluster fit: slow
    DXT_QUALITY_HIGH = 2, // cluster fit (recommended)
    DXT_QUALITY_LOW = 3, // range fit

    DXT_METRIC_PERCEPTUAL = 4, //  for colour error (recommended for diffuse textures)

    DXT_WEIGHT_COLOUR_BY_ALPHA = 8, // Weight the colour by alpha during cluster fit
};

typedef std::vector<ImageBase*> ImageBases;

struct DDSCube {
    ImageBases west, east, south, north, bottom, top;
};

DDSFormat format_from_string (const std::string &str);
std::string format_to_string (DDSFormat fmt);

void dds_save_simple (const std::string &filename, DDSFormat format, const ImageBases &img, int flags);
void dds_save_cube (const std::string &filename, DDSFormat format, const ImageBases &img, int flags);
void dds_save_volume (const std::string &filename, DDSFormat format, const ImageBases &img, int flags);

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

DDSFile dds_open (const std::string &filename);

#endif
