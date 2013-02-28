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

#include <string>
#include <iostream>

#include <FreeImage.h>

#include "Image.h"


Image *image_load (const std::string &filename)
{

    FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename.c_str(), 0);
    if (fif == FIF_UNKNOWN) {
        fif = FreeImage_GetFIFFromFilename(filename.c_str());
    }
    if (fif == FIF_UNKNOWN) {
        std::cerr << "Unknown format." << std::endl;
        fif = FIF_TIFF;
    }
    if (!FreeImage_FIFSupportsReading(fif)) {
        std::cerr << "Unreadable format." << std::endl;
        return NULL;
    }

    FIBITMAP *input = FreeImage_Load(fif, filename.c_str());
    if (input == NULL) {
        return NULL;
    }

    Image *my_image = new Image(FreeImage_GetWidth(input), FreeImage_GetHeight(input));

    for (unsigned y=0 ; y<my_image->height ; ++y) {
        for (unsigned x=0 ; x<my_image->width ; ++x) {
            RGBQUAD pixel;
            FreeImage_GetPixelColor(input, x, y, &pixel);
            my_image->pixel(x, y).r = pixel.rgbRed;
            my_image->pixel(x, y).g = pixel.rgbGreen;
            my_image->pixel(x, y).b = pixel.rgbBlue;
        }
    }
            
    FreeImage_Unload(input);

    return my_image;
}

