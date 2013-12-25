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
        DDSF_R8,
        DDSF_R16,
        DDSF_A8R8,
        DDSF_A16R16,
        DDSF_R3G3B2,
};

typedef std::vector<ImageBase*> ImageBases;

struct DDSCubeFace {
        ImageBases north, south, east, west, top, bottom;
};

DDSFormat format_from_string (const std::string &str);
std::string format_to_string (DDSFormat fmt);

void dds_save_simple (const std::string &filename, DDSFormat format, const ImageBases &img);

#endif
