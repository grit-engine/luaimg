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


#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>

#include <string>
#include <iostream>
#include <fstream>

extern "C" {
	#include <FreeImage.h>
}

#include <colour_conversion.h>

#include "Image.h"
#include "sfi.h"

// supported values: <1,0>, <3,0>, <3,1>, <4,0>
template<chan_t ch, chan_t ach> Image<ch,ach> *image_from_fibitmap (FIBITMAP *input, uimglen_t width, uimglen_t height)
{
    Image<ch,ach> *my_image = new Image<ch,ach>(width, height);

    // from 1 to all of these used, depending on ch+ach
    int channel_offset[4] = { FI_RGBA_RED, FI_RGBA_GREEN, FI_RGBA_BLUE, FI_RGBA_ALPHA };

    for (uimglen_t y=0 ; y<height ; y++) {

        BYTE *raw = FreeImage_GetScanLine(input, y);

        for (uimglen_t x=0 ; x<width ; x++) {

            for (chan_t c=0 ; c<ch+ach ; ++c)
                my_image->pixel(x, y)[c] = raw[channel_offset[c]] / 255.0f;

            raw += ch+ach;
        }
    }

    return my_image;
}
template<> Image<1,0> *image_from_fibitmap<1,0> (FIBITMAP *input, uimglen_t width, uimglen_t height)
{
    Image<1,0> *my_image = new Image<1,0>(width, height);

    for (uimglen_t y=0 ; y<height ; y++) {

        BYTE *raw = FreeImage_GetScanLine(input, y);

        for (uimglen_t x=0 ; x<width ; x++) {
            my_image->pixel(x, y)[0] = raw[x] / 255.0f;
        }
    }

    return my_image;
}
            

template<chan_t ch, chan_t ach> FIBITMAP *image_to_fibitmap (Image<ch,ach> *image, uimglen_t width, uimglen_t height)
{
    FIBITMAP *output = FreeImage_AllocateT(FIT_BITMAP, width, height, (ach+ch)*8, FI_RGBA_RED_MASK, FI_RGBA_BLUE_MASK, FI_RGBA_GREEN_MASK);

    // from 1 to all of these used, depending on ch+ach
    int channel_offset[4] = { FI_RGBA_RED, FI_RGBA_GREEN, FI_RGBA_BLUE, FI_RGBA_ALPHA };

    for (uimglen_t y=0 ; y<height ; y++) {

        BYTE *raw = FreeImage_GetScanLine(output, y);

        for (uimglen_t x=0 ; x<width ; x++) {

            for (chan_t c=0 ; c<ch+ach ; ++c) {
                float v = image->pixel(x, y)[c];
                v = v < 0 ? 0 : v > 1 ? 1 : v;
                raw[channel_offset[c]] = BYTE(v * 255);
            }

            raw += ch+ach;
        }
    }

    return output;
}
template<> FIBITMAP *image_to_fibitmap (Image<1,1> *image, uimglen_t width, uimglen_t height)
{
    FIBITMAP *output = FreeImage_AllocateT(FIT_BITMAP, width, height, 32, FI_RGBA_RED_MASK, FI_RGBA_BLUE_MASK, FI_RGBA_GREEN_MASK);

    for (uimglen_t y=0 ; y<height ; y++) {

        BYTE *raw = FreeImage_GetScanLine(output, y);

        for (uimglen_t x=0 ; x<width ; x++) {

            float v = image->pixel(x, y)[0];
            v = v < 0 ? 0 : v > 1 ? 1 : v;
            raw[FI_RGBA_RED] = BYTE(v * 255);
            raw[FI_RGBA_GREEN] = BYTE(v * 255);
            raw[FI_RGBA_BLUE] = BYTE(v * 255);

            v = image->pixel(x, y)[1];
            v = v < 0 ? 0 : v > 1 ? 1 : v;
            raw[FI_RGBA_ALPHA] = BYTE(v * 255);

            raw += 4;
        }
    }

    return output;
}
template<> FIBITMAP *image_to_fibitmap (Image<2,0> *image, uimglen_t width, uimglen_t height)
{
    FIBITMAP *output = FreeImage_AllocateT(FIT_BITMAP, width, height, 24, FI_RGBA_RED_MASK, FI_RGBA_BLUE_MASK, FI_RGBA_GREEN_MASK);

    for (uimglen_t y=0 ; y<height ; y++) {

        BYTE *raw = FreeImage_GetScanLine(output, y);

        for (uimglen_t x=0 ; x<width ; x++) {

            {
                float v = image->pixel(x, y)[0];
                v = v < 0 ? 0 : v > 1 ? 1 : v;
                raw[FI_RGBA_RED] = BYTE(v * 255);
            }
            {
                float v = image->pixel(x, y)[1];
                v = v < 0 ? 0 : v > 1 ? 1 : v;
                raw[FI_RGBA_GREEN] = BYTE(v * 255);
            }
            {
                raw[FI_RGBA_BLUE] = 0;
            }

            raw += 3;
        }
    }

    return output;
}
template<> FIBITMAP *image_to_fibitmap (Image<1,0> *image, uimglen_t width, uimglen_t height)
{
    FIBITMAP *output = FreeImage_AllocateT(FIT_BITMAP, width, height, 8);

    for (uimglen_t y=0 ; y<height ; y++) {

        BYTE *raw = FreeImage_GetScanLine(output, y);

        for (uimglen_t x=0 ; x<width ; x++) {
            float v = image->pixel(x, y)[0];
            v = v < 0 ? 0 : v > 1 ? 1 : v;
            raw[x] = BYTE(v * 255);
        }
    }

    return output;
}




ImageBase *image_load (const std::string &filename)
{
    size_t dot = filename.rfind('.');
    if (dot == std::string::npos) {
        std::cerr << "No file extension." << std::endl;
        return NULL;
    }
    std::string ext = filename.substr(dot+1);
    if (ext == "") {
        std::cerr << "No file extension." << std::endl;
        return NULL;
    }


    if (ext == "sfi") {

        return sfi_open(filename);

    } else {

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
        uimglen_t width = FreeImage_GetWidth(input);
        uimglen_t height = FreeImage_GetHeight(input);

        switch (input_type) {
            case FIT_BITMAP: {

                // how many channels?
                int bits = FreeImage_GetBPP(input);

                if (FreeImage_GetColorsUsed(input) != 0 && bits != 8) {
                    std::cerr << "Images with palettes not supported when number of bits is" << bits << "." << std::endl;
                    FreeImage_Unload(input);
                    return NULL;
                }
            
                switch (bits) {
                    case 1:
                    std::cerr << "Images with 1 bit colour not supported." << std::endl;
                    FreeImage_Unload(input);
                    return NULL;

                    case 4:
                    std::cerr << "Images with 4 bit colour not supported." << std::endl;
                    FreeImage_Unload(input);
                    return NULL;

                    case 8: {
                        ImageBase *my_image = image_from_fibitmap<1,0>(input, width, height);
                        FreeImage_Unload(input);
                        return my_image;
                    }
                        
                    case 16: {
                        ImageBase *my_image = image_from_fibitmap<3,0>(input, width, height);
                        FreeImage_Unload(input);
                        return my_image;
                    }
                        
                    case 24: {
                        ImageBase *my_image = image_from_fibitmap<3,0>(input, width, height);
                        FreeImage_Unload(input);
                        return my_image;
                    }
                        
                    case 32: {
                        ImageBase *my_image = image_from_fibitmap<3,1>(input, width, height);
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
}

bool image_save (ImageBase *image, const std::string &filename)
{
    size_t dot = filename.rfind('.');
    if (dot == std::string::npos) {
        std::cerr << "No file extension." << std::endl;
        return false;
    }
    std::string ext = filename.substr(dot+1);
    if (ext == "") {
        std::cerr << "No file extension." << std::endl;
        return false;
    }

    uimglen_t width = image->width;
    uimglen_t height = image->height;
    chan_t channels = image->channels();

    if (ext == "sfi") {

        std::ofstream out;
        out.open(filename);
        if (!out.good()) {
            std::cerr<<filename<<": "<<std::string(strerror(errno))<<std::endl;
            return false;
        }

        out.write((char*)&width, sizeof(width));
        out.write((char*)&height, sizeof(height));
        out.write((char*)&channels, sizeof(channels));
        out << (image->hasAlpha() ? 'A' : 'a');
        float *raw = image->raw();
        for (size_t i=0 ; i<width*height*channels ; ++i) {
            out.write((char*)&raw[i], sizeof(float));
        }
        if (!out.good()) {
            std::cerr<<filename<<": "<<std::string(strerror(errno))<<std::endl;
            return false;
        }
        out.close();

        return true;
        
    } else {

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

        // make new fibitmap as a copy of image
        FIBITMAP *output;
        switch (channels) {
            case 1:
            if (image->hasAlpha()) {
                output = image_to_fibitmap(static_cast<Image<0,1>*>(image), width, height);
            } else {
                output = image_to_fibitmap(static_cast<Image<1,0>*>(image), width, height);
            }
            break;

            case 2:
            if (image->hasAlpha()) {
                output = image_to_fibitmap(static_cast<Image<1,1>*>(image), width, height);
            } else {
                output = image_to_fibitmap(static_cast<Image<2,0>*>(image), width, height);
            }
            break;

            case 3:
            if (image->hasAlpha()) {
                output = image_to_fibitmap(static_cast<Image<2,1>*>(image), width, height);
            } else {
                output = image_to_fibitmap(static_cast<Image<3,0>*>(image), width, height);
            }
            break;

            case 4:
            if (image->hasAlpha()) {
                output = image_to_fibitmap(static_cast<Image<3,1>*>(image), width, height);
            } else {
                output = image_to_fibitmap(static_cast<Image<4,0>*>(image), width, height);
            }
            break;

            default:
            std::cerr << "Can only save images with 1, 2, 3, or 4 channels." << std::endl;
            return false;
        }

        // save it
        bool status = 0!=FreeImage_Save(fif, output, filename.c_str());

        FreeImage_Unload(output);

        return status;
    }
}

FREE_IMAGE_FILTER to_fi (ScaleFilter sf)
{
    switch (sf) {
        case SF_BOX: return FILTER_BOX;
        case SF_BILINEAR: return FILTER_BILINEAR;
        case SF_BSPLINE: return FILTER_BSPLINE;
        case SF_BICUBIC: return FILTER_BICUBIC;
        case SF_CATMULLROM: return FILTER_CATMULLROM;
        case SF_LANCZOS3: return FILTER_LANCZOS3;
        default: return FILTER_BOX;
    }
}

template<chan_t ch, chan_t ach> FIBITMAP *image_to_fifloat (const ImageBase *img_)
{
    const Image<ch,ach> *img = static_cast<const Image<ch,ach>*>(img_);
    FIBITMAP *fib = NULL;
    uimglen_t actual_channels = 0;
    switch (ch+ach) {
        case 4:
        actual_channels = 4;
        fib = FreeImage_AllocateT(FIT_RGBAF, img->width, img->height);
        break;

        case 3:
        case 2:
        actual_channels = 3;
        fib = FreeImage_AllocateT(FIT_RGBF, img->width, img->height);
        break;

        case 1:
        actual_channels = 1;
        fib = FreeImage_AllocateT(FIT_FLOAT, img->width, img->height);
        break;
    }
    
    for (uimglen_t y=0 ; y<img->height ; y++) {

        float *raw = reinterpret_cast<float*>(FreeImage_GetScanLine(fib, y));

        for (uimglen_t x=0 ; x<img->width ; x++) {
            for (uimglen_t c=0 ; c<img->channels(); ++c) {
                float v = img->pixel(x, y)[c];
                raw[x*actual_channels + c] = v;
            }
        }
    }

    return fib;

}

template<chan_t ch, chan_t ach> Image<ch,ach> *image_from_fifloat (FIBITMAP *img)
{
    uimglen_t actual_channels = 0;
    switch (ch+ach) {
        case 4:
        actual_channels = 4;
        break;

        case 3:
        case 2:
        actual_channels = 3;
        break;

        case 1:
        actual_channels = 1;
        break;
    }

    uimglen_t width = FreeImage_GetWidth(img);
    uimglen_t height = FreeImage_GetHeight(img);
    
    Image<ch, ach> *output = new Image<ch, ach>(width, height);
    for (uimglen_t y=0 ; y<height ; ++y) {

        float *raw = reinterpret_cast<float*>(FreeImage_GetScanLine(img, y));

        for (uimglen_t x=0 ; x<width ; ++x) {
            for (chan_t c=0 ; c<output->channels() ; ++c) {
                float v = raw[x*actual_channels + c];
                output->pixel(x, y)[c] = v;
            }
        }
    }

    return output;
}


template<chan_t ch, chan_t ach>
ImageBase *do_scale (const ImageBase *src, uimglen_t dst_width, uimglen_t dst_height, ScaleFilter filter)
{
    FIBITMAP *fib = image_to_fifloat<ch,ach>(src);
    
    FIBITMAP *scaled = FreeImage_Rescale(fib, dst_width, dst_height, to_fi(filter));
    FreeImage_Unload(fib);

    ImageBase *r = image_from_fifloat<ch,ach>(scaled);
    FreeImage_Unload(scaled);
    return r;
}

ImageBase *ImageBase::scale (uimglen_t dst_width, uimglen_t dst_height, ScaleFilter filter) const
{
    switch (channels()) {
        case 1: return hasAlpha() ? do_scale<0,1>(this, dst_width, dst_height, filter)
                                  : do_scale<1,0>(this, dst_width, dst_height, filter);
        case 2: return hasAlpha() ? do_scale<1,1>(this, dst_width, dst_height, filter)
                                  : do_scale<2,0>(this, dst_width, dst_height, filter);
        case 3: return hasAlpha() ? do_scale<2,1>(this, dst_width, dst_height, filter)
                                  : do_scale<3,0>(this, dst_width, dst_height, filter);
        case 4: return hasAlpha() ? do_scale<3,1>(this, dst_width, dst_height, filter)
                                  : do_scale<4,0>(this, dst_width, dst_height, filter);
        default: return NULL;
    }
}
