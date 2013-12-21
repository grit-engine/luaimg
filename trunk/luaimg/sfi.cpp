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

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>

#include <exception.h>
#include <io_util.h>

#include "sfi.h"

void sfi_save (const std::string &filename, ImageBase *image)
{

    uimglen_t width = image->width;
    uimglen_t height = image->height;
    chan_t channels = image->channels();

    std::ofstream out;
    io_util_open(filename, out);
    io_util_write(filename, out, width);
    io_util_write(filename, out, height);
    io_util_write(filename, out, channels);

    out << (image->hasAlpha() ? 'A' : 'a');

    float *raw = image->raw();
    for (size_t i=0 ; i<width*height*channels ; ++i) {
        io_util_write(filename, out, raw[i]);
    }
    out.close();
}

ImageBase *sfi_open (const std::string &filename)
{
    std::ifstream in;
    io_util_open(filename, in);

    uimglen_t width, height;
    chan_t channels;
    io_util_read(filename, in, width);
    io_util_read(filename, in, height);
    io_util_read(filename, in, channels);

    char alpha_char;
    io_util_read(filename, in, alpha_char);
    bool has_alpha;
    if (alpha_char == 'A') {
        has_alpha = true;
    } else if (alpha_char == 'a') {
        has_alpha = false;
    } else {
        EXCEPT<<filename<<": corrupted image file"<<std::endl;
    }

    ImageBase *img = NULL;

    try {
        switch (channels) {
            case 1: if (has_alpha) {
                img = new Image<0,1>(width,height);
            } else {
                img = new Image<1,0>(width,height);
            } break;
            case 2: if (has_alpha) {
                img = new Image<1,1>(width,height);
            } else {
                img = new Image<2,0>(width,height);
            } break;
            case 3: if (has_alpha) {
                img = new Image<2,1>(width,height);
            } else {
                img = new Image<3,0>(width,height);
            } break;
            case 4: if (has_alpha) {
                img = new Image<3,1>(width,height);
            } else {
                img = new Image<4,0>(width,height);
            } break;
            default:
            EXCEPT<<filename<<": corrupted image file"<<std::endl;
        }

        float *raw = img->raw();
        for (size_t i=0 ; i<width*height*channels ; ++i) {
            io_util_read(filename, in, raw[i]);
        }

    } catch (const Exception &e) {
        delete img;
        throw e;
    }

    in.close();

    return img;
}
