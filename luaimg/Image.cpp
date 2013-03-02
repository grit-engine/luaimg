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

#include <cmath>

#include <string>
#include <iostream>

#include <FreeImage.h>

#include "Image.h"

// ch may not be more than 4!
template<unsigned ch> Image<ch> *image_from_fibitmap (FIBITMAP *input, unsigned width, unsigned height)
{
    Image<ch> *my_image = new Image<ch>(width, height);

    // from 1 to all of these used, depending on ch
    unsigned channel_offset[4] = { FI_RGBA_RED, FI_RGBA_GREEN, FI_RGBA_BLUE, FI_RGBA_ALPHA };

    for (unsigned y=0 ; y<height ; y++) {

        BYTE *raw = FreeImage_GetScanLine(input, y);

        for (unsigned x=0 ; x<width ; x++) {

            for (unsigned c=0 ; c<ch ; ++c)
                my_image->pixel(x, y)[c] = raw[channel_offset[c]] / 255.0f;

            raw += ch;
        }
    }

    return my_image;
}
            

template<unsigned ch> FIBITMAP *image_to_fibitmap (Image<ch> *image, unsigned width, unsigned height)
{
    FIBITMAP *output = FreeImage_AllocateT(FIT_BITMAP, width, height, ch*8, FI_RGBA_RED_MASK, FI_RGBA_BLUE_MASK, FI_RGBA_GREEN_MASK);

    // from 1 to all of these used, depending on ch
    unsigned channel_offset[4] = { FI_RGBA_RED, FI_RGBA_GREEN, FI_RGBA_BLUE, FI_RGBA_ALPHA };

    for (unsigned y=0 ; y<height ; y++) {

        BYTE *raw = FreeImage_GetScanLine(output, y);

        for (unsigned x=0 ; x<width ; x++) {

            for (unsigned c=0 ; c<ch ; ++c) {
                float v = image->pixel(x, y)[c];
                v = v < 0 ? 0 : v > 1 ? 1 : v;
                raw[channel_offset[c]] = BYTE(v * 255);
            }

            raw += ch;
        }
    }

    return output;
}




ImageBase *image_load (const std::string &filename)
{

    FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename.c_str(), 0);
    if (fif == FIF_UNKNOWN) {
        fif = FreeImage_GetFIFFromFilename(filename.c_str());
    }
    if (fif == FIF_UNKNOWN) {
        std::cerr << "Unknown format." << std::endl;
        return NULL;
    }
    if (!FreeImage_FIFSupportsReading(fif)) {
        std::cerr << "Unreadable format." << std::endl;
        return NULL;
    }

    FIBITMAP *input = FreeImage_Load(fif, filename.c_str());
    if (input == NULL) {
        return NULL;
    }

    FREE_IMAGE_TYPE input_type = FreeImage_GetImageType(input);
    unsigned width = FreeImage_GetWidth(input);
    unsigned height = FreeImage_GetHeight(input);

    switch (input_type) {
        case FIT_BITMAP: {

            if (FreeImage_GetColorsUsed(input) != 0) {
                std::cerr << "Images with palettes not supported." << std::endl;
                FreeImage_Unload(input);
                return NULL;
            }
        
            // how many channels?
            unsigned bits = FreeImage_GetBPP(input);
            switch (bits) {
                case 1:
                std::cerr << "Images with 1 bit colour not supported." << std::endl;
                FreeImage_Unload(input);
                return NULL;

                case 4:
                std::cerr << "Images with 1 bit colour not supported." << std::endl;
                FreeImage_Unload(input);
                return NULL;

                case 8: {
                    ImageBase *my_image = image_from_fibitmap<1>(input, width, height);
                    FreeImage_Unload(input);
                    return my_image;
                }
                    
                case 16: {
                    ImageBase *my_image = image_from_fibitmap<2>(input, width, height);
                    FreeImage_Unload(input);
                    return my_image;
                }
                    
                case 24: {
                    ImageBase *my_image = image_from_fibitmap<3>(input, width, height);
                    FreeImage_Unload(input);
                    return my_image;
                }
                    
                case 32: {
                    ImageBase *my_image = image_from_fibitmap<4>(input, width, height);
                    FreeImage_Unload(input);
                    return my_image;
                }
                
                default:
                std::cerr << "Only 8, 16, 24, and 32 bit images supported (not "<<bits<<")." << std::endl;
                FreeImage_Unload(input);
                return NULL;
            }

        }
                    
        case FIT_UINT16:
        std::cerr << "Unsupported image type: UINT16." << std::endl;
        FreeImage_Unload(input);
        return NULL;

        case FIT_INT16:
        std::cerr << "Unsupported image type: INT16." << std::endl;
        FreeImage_Unload(input);
        return NULL;

        case FIT_UINT32:
        std::cerr << "Unsupported image type: UINT32." << std::endl;
        FreeImage_Unload(input);
        return NULL;

        case FIT_INT32:
        std::cerr << "Unsupported image type: INT32." << std::endl;
        FreeImage_Unload(input);
        return NULL;

        case FIT_FLOAT:
        std::cerr << "Unsupported image type: FLOAT." << std::endl;
        FreeImage_Unload(input);
        return NULL;

        case FIT_DOUBLE:
        std::cerr << "Unsupported image type: DOUBLE." << std::endl;
        FreeImage_Unload(input);
        return NULL;

        case FIT_COMPLEX:
        std::cerr << "Unsupported image type: COMPLEX." << std::endl;
        FreeImage_Unload(input);
        return NULL;

        case FIT_RGB16:
        std::cerr << "Unsupported image type: RGB16." << std::endl;
        FreeImage_Unload(input);
        return NULL;

        case FIT_RGBA16:
        std::cerr << "Unsupported image type: RGBA16." << std::endl;
        FreeImage_Unload(input);
        return NULL;

        case FIT_RGBF:
        std::cerr << "Unsupported image type: RGBF." << std::endl;
        FreeImage_Unload(input);
        return NULL;

        case FIT_RGBAF:
        std::cerr << "Unsupported image type: RGBAF." << std::endl;
        FreeImage_Unload(input);
        return NULL;

        case FIT_UNKNOWN:
        default:
        std::cerr << "Unknown type." << std::endl;
        FreeImage_Unload(input);
        return NULL;
    }
}

bool image_save (ImageBase *image, const std::string &filename)
{

    FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename.c_str(), 0);
    if (fif == FIF_UNKNOWN) {
        fif = FreeImage_GetFIFFromFilename(filename.c_str());
    }
    if (fif == FIF_UNKNOWN) {
        std::cerr << "Unknown format." << std::endl;
        return false;
    }
    if (!FreeImage_FIFSupportsWriting(fif)) {
        std::cerr << "Unwritable format." << std::endl;
        return false;
    }

    unsigned width = image->width;
    unsigned height = image->width;
    unsigned channels = image->channels();

    // make new fibitmap as a copy of image
    FIBITMAP *output;
    switch (channels) {
        case 1:
        output = image_to_fibitmap<1>(static_cast<Image<1>*>(image), width, height);
        break;

        case 2:
        output = image_to_fibitmap<2>(static_cast<Image<2>*>(image), width, height);
        break;

        case 3:
        output = image_to_fibitmap<3>(static_cast<Image<3>*>(image), width, height);
        break;

        case 4:
        output = image_to_fibitmap<4>(static_cast<Image<4>*>(image), width, height);
        break;

        default:
        std::cerr << "Can only save images with 1, 2, 3, or 4 channels." << std::endl;
        return false;
    }

    // save it
    bool status = FreeImage_Save(fif, output, filename.c_str());

    FreeImage_Unload(output);

    return status;
}

