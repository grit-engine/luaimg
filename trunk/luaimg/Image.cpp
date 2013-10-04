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
#include <cassert>
#include <cstring>

#include <string>
#include <iostream>
#include <fstream>

extern "C" {
	#include <FreeImage.h>
}

#include "Image.h"

void RGBtoHSL (float R, float G, float B, float &H, float &S, float &L)
{

    float max_intensity = std::max(std::max(R,G),B);
    float min_intensity = std::min(std::min(R,G),B);

    float delta = max_intensity - min_intensity;

    L = 0.5f * (max_intensity + min_intensity);

    if (delta == 0) {
        // all channels the same (colour is grey)
        S = 0.0f;
        H = 0.0f;
        return;
    }

    if (L < 0.5f) {
        S = (max_intensity - min_intensity)/(max_intensity + min_intensity);
    } else {
        S = (max_intensity - min_intensity)/(2 - max_intensity - min_intensity);
    }
    if (max_intensity == R) {
        H = (G - B)/delta;
    }
    if (max_intensity == G) {
        H = 2 + (B - R)/delta;
    }
    if (max_intensity == B) {
        H = 4 + (R - G)/delta;
    }
    H /= 6;
    if (H < 0) H += 1;
}

static float HSLtoRGB_aux (float temp1, float temp2, float temp3)
{
    if (temp3 < 1)      return temp2  +  (temp1-temp2) * temp3;
    else if (temp3 < 3) return temp1;
    else if (temp3 < 4) return temp2  +  (temp1-temp2) * (4 - temp3);
    else                return temp2;
}

void HSLtoRGB (float H, float S, float L, float &R, float &G, float &B)
{
    if (S == 0) {
        // grey
        R = L;
        G = L;
        B = L;
        return;
    }

    float temp1 = L<0.5f ? L + L*S : L + S - L*S;
    float temp2 = 2*L - temp1;

    R = HSLtoRGB_aux(temp1, temp2, fmodf(6*H + 2, 6));
    G = HSLtoRGB_aux(temp1, temp2, 6*H);
    B = HSLtoRGB_aux(temp1, temp2, fmodf(6*H + 4, 6));
}

void HSVtoHSL (float h, float s, float v, float &hh, float &ss, float &ll)
{
    hh = h;
    ll = (2 - s) * v;
    ss = s * v;
    ss /= (ll <= 1) ? ll : 2 - ll;
    ll /= 2;
}

void HSLtoHSV (float hh, float ss, float ll, float &h, float &s, float &v)
{
    h = hh;
    ss *= (ll <= 0.5) ? ll : 1 - ll;
    v = ll + ss;
    s = 2 * ss / (ll + ss);
}

void RGBtoHSV (float R, float G, float B, float &H, float &S, float &V)
{
    float max_intensity = std::max(std::max(R,G),B);
    float min_intensity = std::min(std::min(R,G),B);

    V = max_intensity;

    float delta = max_intensity - min_intensity;

    if (delta == 0) {
        // grey
        H = 0;
        S = 0;
        return;
    }

    S = (delta / max_intensity);

    if (max_intensity == R) {
        H = (G - B)/delta;
    }
    if (max_intensity == G) {
        H = 2 + (B - R)/delta;
    }
    if (max_intensity == B) {
        H = 4 + (R - G)/delta;
    }
    H /= 6;
    if (H < 0) H += 1;
}


void HSVtoRGB (float H, float S, float V, float &R, float &G, float &B)
{
    if (S == 0.0) {
        // grey
        R = V;
        G = V;
        B = V;
        return;
    }

    float hh = fmodf(H * 6, 6);
    long i = (long)hh;
    float ff = hh - i;
    float p = V * (1.0 - S);
    float q = V * (1.0 - (S * ff));
    float t = V * (1.0 - (S * (1.0 - ff)));

    switch (i) {

        case 0:
        R = V;
        G = t;
        B = p;
        break;

        case 1:
        R = q;
        G = V;
        B = p;
        break;

        case 2:
        R = p;
        G = V;
        B = t;
        break;

        case 3:
        R = p;
        G = q;
        B = V;
        break;

        case 4:
        R = t;
        G = p;
        B = V;
        break;

        case 5:
        default:
        R = V;
        G = p;
        B = q;
        break;
    }
}


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
template<> FIBITMAP *image_to_fibitmap (Image<0,1> *image, uimglen_t width, uimglen_t height)
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

        std::ifstream in;
        in.open(filename);
        if (!in.good()) {
            std::cerr<<filename<<": "<<std::string(strerror(errno))<<std::endl;
            return NULL;
        }


        uimglen_t width, height;
        in.read((char*)&width, sizeof(width));
        if (!in.good()) {
            std::cerr<<filename<<": "<<std::string(strerror(errno))<<std::endl;
            return NULL;
        }
        in.read((char*)&height, sizeof(height));
        if (!in.good()) {
            std::cerr<<filename<<": "<<std::string(strerror(errno))<<std::endl;
            return NULL;
        }
        chan_t channels;
        in.read((char*)&channels, sizeof(channels));
        if (!in.good()) {
            std::cerr<<filename<<": "<<std::string(strerror(errno))<<std::endl;
            return NULL;
        }
        char alpha_char;
        in.read(&alpha_char, 1);
        if (!in.good()) {
            std::cerr<<filename<<": "<<std::string(strerror(errno))<<std::endl;
            return NULL;
        }

        bool has_alpha;
        if (alpha_char == 'A') {
            has_alpha = true;
        } else if (alpha_char == 'a') {
            has_alpha = false;
        } else {
            std::cerr<<filename<<": corrupted image file"<<std::endl;
            return NULL;
        }

        ImageBase *img = NULL;

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
            std::cerr<<filename<<": corrupted image file"<<std::endl;
            return NULL;
        }

        float *raw = img->raw();
        for (size_t i=0 ; i<width*height*channels ; ++i) {
            in.read((char*)&raw[i], sizeof(float));
        }

        if (!in.good()) {
            std::cerr<<filename<<": "<<std::string(strerror(errno))<<std::endl;
            delete img;
            return NULL;
        }

        in.close();

        return img;

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

template<chan_t ch, chan_t ach>
ImageBase *do_rotate (const ImageBase *src, float angle)
{
    FIBITMAP *fib = image_to_fifloat<ch,ach>(src);
    
    FIBITMAP *rotated = FreeImage_Rotate(fib, angle, NULL);
    FreeImage_Unload(fib);

    ImageBase *r = image_from_fifloat<ch,ach>(rotated);
    FreeImage_Unload(rotated);
    return r;
}

ImageBase *ImageBase::rotate (float angle) const
{
    switch (channels()) {
        case 1: return hasAlpha() ? do_rotate<0,1>(this, angle) : do_rotate<1,0>(this, angle);
        case 2: return hasAlpha() ? do_rotate<1,1>(this, angle) : do_rotate<2,0>(this, angle);
        case 3: return hasAlpha() ? do_rotate<2,1>(this, angle) : do_rotate<3,0>(this, angle);
        case 4: return hasAlpha() ? do_rotate<3,1>(this, angle) : do_rotate<4,0>(this, angle);
        default: return NULL;
    }
}
