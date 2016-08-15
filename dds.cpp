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


/* Reference material for this code:
 *
 * BC1-3 (DXT1,3,5): http://en.wikipedia.org/wiki/S3_Texture_Compression
 *
 * BC4-5 (ATI1,2): http://en.wikipedia.org/wiki/3Dc
 *
 * High level stuff about current and future formats:
 *     http://www.reedbeta.com/blog/2012/02/12/understanding-bcn-texture-compression-formats/
 *
 * MSDN resources: http://msdn.microsoft.com/en-us/library/windows/desktop/bb943990(v=vs.85).aspx
 */

#include <cstdlib>
#include <cstdint>

#include <squish.h>

#include <io_util.h>

#include "dds.h"

#define DDSD_CAPS 0x1
#define DDSD_HEIGHT 0x2
#define DDSD_WIDTH 0x4
#define DDSD_PITCH 0x8
#define DDSD_PIXELFORMAT 0x1000
#define DDSD_MIPMAPCOUNT 0x20000
#define DDSD_LINEARSIZE 0x80000
#define DDSD_DEPTH 0x800000

#define DDSCAPS_COMPLEX 0x8
#define DDSCAPS_MIPMAP 0x400000
#define DDSCAPS_TEXTURE 0x1000

#define DDSCAPS2_CUBEMAP 0x200
#define DDSCAPS2_CUBEMAP_POSITIVEX 0x400
#define DDSCAPS2_CUBEMAP_NEGATIVEX 0x800
#define DDSCAPS2_CUBEMAP_POSITIVEY 0x1000
#define DDSCAPS2_CUBEMAP_NEGATIVEY 0x2000
#define DDSCAPS2_CUBEMAP_POSITIVEZ 0x4000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ 0x8000
#define DDSCAPS2_VOLUME 0x200000

#define DDPF_ALPHAPIXELS 0x1
#define DDPF_ALPHA 0x2
#define DDPF_FOURCC 0x4
#define DDPF_RGB 0x40
#define DDPF_YUV 0x200
#define DDPF_LUMINANCE 0x20000

#define FOURCC(x,y,z,w) uint32_t(((w)<<24) | ((z)<<16) | ((y)<<8) | (x))


DDSFormat format_from_string (const std::string &str)
{
    if (str == "R5G6B5") return DDSF_R5G6B5;
    else if (str == "R8G8B8") return DDSF_R8G8B8;
    else if (str == "A8R8G8B8") return DDSF_A8R8G8B8;
    else if (str == "A2R10G10B10") return DDSF_A2R10G10B10;
    else if (str == "A1R5G5B5") return DDSF_A1R5G5B5;
    else if (str == "R8") return DDSF_R8;
    else if (str == "G16R16") return DDSF_G16R16;
    else if (str == "R16") return DDSF_R16;
    else if (str == "A8R8") return DDSF_A8R8;
    else if (str == "A4R4") return DDSF_A4R4;
    else if (str == "A16R16") return DDSF_A16R16;
    else if (str == "R3G3B2") return DDSF_R3G3B2;
    else if (str == "A4R4G4B4") return DDSF_A4R4G4B4;
    else if (str == "BC1") return DDSF_BC1;
    else if (str == "BC2") return DDSF_BC2;
    else if (str == "BC3") return DDSF_BC3;
    else if (str == "BC4") return DDSF_BC4;
    else if (str == "BC5") return DDSF_BC5;
    else if (str == "R16F") return DDSF_R16F;
    else if (str == "G16R16F") return DDSF_G16R16F;
    else if (str == "R16G16B16A16F") return DDSF_R16G16B16A16F;
    else if (str == "R32F") return DDSF_R32F;
    else if (str == "G32R32F") return DDSF_G32R32F;
    else if (str == "R32G32B32A32F") return DDSF_R32G32B32A32F;
    else {
        EXCEPT << "Unrecognised DDS Format: " << str << ENDL;
    }
}

std::string format_to_string (DDSFormat format)
{
    switch (format) {
        case DDSF_R5G6B5: return "R5G6B5";
        case DDSF_R8G8B8: return "R8G8B8";
        case DDSF_A8R8G8B8: return "A8R8G8B8";
        case DDSF_A2R10G10B10: return "A2R10G10B10";
        case DDSF_A1R5G5B5: return "A1R5G5B5";
        case DDSF_R8: return "R8";
        case DDSF_R16: return "R16";
        case DDSF_G16R16: return "G16R16";
        case DDSF_A8R8: return "A8R8";
        case DDSF_A4R4: return "A4R4";
        case DDSF_A16R16: return "A16R16";
        case DDSF_R3G3B2: return "R3G3B2";
        case DDSF_A4R4G4B4: return "A4R4G4B4";
        case DDSF_BC1: return "BC1";
        case DDSF_BC2: return "BC2";
        case DDSF_BC3: return "BC3";
        case DDSF_BC4: return "BC4";
        case DDSF_BC5: return "BC5";
        case DDSF_R16F: return "R16F";
        case DDSF_G16R16F: return "G16R16F";
        case DDSF_R16G16B16A16F: return "R16G16B16A16F";
        case DDSF_R32F: return "R32F";
        case DDSF_G32R32F: return "G32R32F";
        case DDSF_R32G32B32A32F: return "R32G32B32A32F";
        default: EXCEPTEX << format << ENDL;
    }
}

namespace {

    void check_colour (DDSFormat format, chan_t ch, bool alpha)
    {
        switch (format) {
            case DDSF_R16F:
            case DDSF_R32F:
            case DDSF_BC4:
            if (ch==1 && !alpha) return;
            break;
            case DDSF_G16R16F:
            case DDSF_G32R32F:
            case DDSF_BC5:
            if (ch==2 && !alpha) return;
            break;
            case DDSF_R16G16B16A16F:
            case DDSF_R32G32B32A32F:
            case DDSF_A2R10G10B10:
            case DDSF_A1R5G5B5:
            case DDSF_A8R8G8B8:
            case DDSF_A4R4G4B4:
            case DDSF_BC1:
            case DDSF_BC2:
            case DDSF_BC3:
            if (ch==3 && alpha) return;
            break;
            case DDSF_R8G8B8:
            case DDSF_R5G6B5:
            case DDSF_R3G3B2:
            if (ch==3 && !alpha) return;
            break;
            case DDSF_G16R16:
            if (ch==2 && !alpha) return;
            break;
            case DDSF_R16:
            case DDSF_R8:
            if (ch==1 && !alpha) return;
            break;
            case DDSF_A16R16:
            case DDSF_A8R8:
            case DDSF_A4R4:
            if (ch==1 && alpha) return;
            break;
            default: EXCEPTEX << format << ENDL;
        }
        EXCEPT << "Image channels do not match desired format: " << format_to_string(format) << ENDL;
    }

    uint32_t bits_per_pixel (DDSFormat format)
    {
        switch (format) {

            case DDSF_BC1:
            case DDSF_BC4:
            return 4;

            case DDSF_BC2:
            case DDSF_BC3:
            case DDSF_BC5:
            case DDSF_R8:
            case DDSF_R3G3B2:
            case DDSF_A4R4:
            return 8;

            case DDSF_R16F:
            case DDSF_R16:
            case DDSF_A8R8:
            case DDSF_R5G6B5:
            case DDSF_A1R5G5B5:
            case DDSF_A4R4G4B4:
            return 16;

            case DDSF_R8G8B8:
            return 24;

            case DDSF_R32F:
            case DDSF_G16R16F:
            case DDSF_A16R16:
            case DDSF_A8R8G8B8:
            case DDSF_A2R10G10B10:
            case DDSF_G16R16:
            return 32;

            case DDSF_R16G16B16A16F:
            case DDSF_G32R32F:
            return 64;

            case DDSF_R32G32B32A32F:
            return 128;

            default: EXCEPTEX << format << ENDL;
        }
    }

    bool is_compressed (DDSFormat format)
    {
        switch (format) {
            case DDSF_BC1:
            case DDSF_BC2:
            case DDSF_BC3:
            case DDSF_BC4:
            case DDSF_BC5:
            return true;
            default: return false;
        }
    }

    void output_pixelformat (OutFile &out, DDSFormat format)
    {
        // DDS_HEADER.PIXELFORMAT
        uint32_t flags = 0;
        uint32_t fourcc = 0;
        uint32_t rgb_bitcount = bits_per_pixel(format);
        uint32_t r_mask = 0;
        uint32_t g_mask = 0;
        uint32_t b_mask = 0;
        uint32_t a_mask = 0;
        switch (format) {
            case DDSF_R5G6B5:
            flags = DDPF_RGB;
            r_mask = 0x0000f800;
            g_mask = 0x000007e0;
            b_mask = 0x0000001f;
            break;
            case DDSF_R8G8B8:
            flags = DDPF_RGB;
            r_mask = 0x00ff0000;
            g_mask = 0x0000ff00;
            b_mask = 0x000000ff;
            break;
            case DDSF_A8R8G8B8:
            flags = DDPF_ALPHAPIXELS | DDPF_RGB;
            r_mask = 0x00ff0000;
            g_mask = 0x0000ff00;
            b_mask = 0x000000ff;
            a_mask = 0xff000000;
            break;
            case DDSF_A2R10G10B10:
            flags = DDPF_ALPHAPIXELS | DDPF_RGB;
            r_mask = 0x3ff00000;
            g_mask = 0x000ffc00;
            b_mask = 0x000003ff;
            a_mask = 0xc0000000;
            break;
            case DDSF_A1R5G5B5:
            flags = DDPF_ALPHAPIXELS | DDPF_RGB;
            r_mask = 0x00007c00;
            g_mask = 0x000003e0;
            b_mask = 0x0000001f;
            a_mask = 0x00008000;
            break;
            case DDSF_R8:
            flags = DDPF_RGB;
            r_mask = 0x000000ff;
            g_mask = 0x00000000;
            b_mask = 0x00000000;
            a_mask = 0x00000000;
            break;
            case DDSF_R16:
            flags = DDPF_RGB;
            r_mask = 0x0000ffff;
            g_mask = 0x00000000;
            b_mask = 0x00000000;
            a_mask = 0x00000000;
            break;
            case DDSF_G16R16:
            flags = DDPF_RGB;
            r_mask = 0xffff0000;
            g_mask = 0x0000ffff;
            b_mask = 0x00000000;
            a_mask = 0x00000000;
            break;
            case DDSF_A8R8:
            flags = DDPF_ALPHAPIXELS | DDPF_RGB;
            r_mask = 0x000000ff;
            g_mask = 0x00000000;
            b_mask = 0x00000000;
            a_mask = 0x0000ff00;
            break;
            case DDSF_A4R4:
            flags = DDPF_ALPHAPIXELS | DDPF_RGB;
            r_mask = 0x0000000f;
            g_mask = 0x00000000;
            b_mask = 0x00000000;
            a_mask = 0x000000f0;
            break;
            case DDSF_A16R16:
            flags = DDPF_ALPHAPIXELS | DDPF_RGB;
            r_mask = 0x0000ffff;
            g_mask = 0x00000000;
            b_mask = 0x00000000;
            a_mask = 0xffff0000;
            break;
            case DDSF_R3G3B2:
            flags = DDPF_RGB;
            r_mask = 0x000000e0;
            g_mask = 0x0000001c;
            b_mask = 0x00000003;
            break;
            case DDSF_A4R4G4B4:
            flags = DDPF_ALPHAPIXELS | DDPF_RGB;
            r_mask = 0x00000f00;
            g_mask = 0x000000f0;
            b_mask = 0x0000000f;
            a_mask = 0x0000f000;
            break;
            case DDSF_BC1:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = FOURCC('D','X','T','1');
            break;
            case DDSF_BC2:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = FOURCC('D','X','T','3');
            break;
            case DDSF_BC3:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = FOURCC('D','X','T','5');
            break;
            case DDSF_BC4:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = FOURCC('A','T','I','1');
            break;
            case DDSF_BC5:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = FOURCC('A','T','I','2');
            break;
            case DDSF_R16F:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = 0x6f;
            break;
            case DDSF_G16R16F:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = 0x70;
            break;
            case DDSF_R16G16B16A16F:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = 0x71;
            break;
            case DDSF_R32F:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = 0x72;
            break;
            case DDSF_G32R32F:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = 0x73;
            break;
            case DDSF_R32G32B32A32F:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = 0x74;
            break;
            default: EXCEPTEX << format << ENDL;
        }
        out.write(uint32_t(32));
        out.write(flags);
        out.write(fourcc);
        out.write(rgb_bitcount);
        out.write(r_mask);
        out.write(g_mask);
        out.write(b_mask);
        out.write(a_mask);
    }

    template<class T> T to_range (float v, unsigned max)
    {
        if (v<0) v = 0;
        if (v>1) v = 1;
        return v * max + 0.5;
    }

    template<chan_t ch, chan_t ach> void write_colour (OutFile &out, DDSFormat format, const Colour<ch,ach> &col)
    {
        switch (format) {
            case DDSF_R5G6B5: {
                uint16_t word = 0;
                word |= to_range<unsigned>(col[0], 31) << 11;
                word |= to_range<unsigned>(col[1], 63) << 5;
                word |= to_range<unsigned>(col[2], 31) << 0;
                out.write(word);
                break;
            }
            case DDSF_R8G8B8:
            out.write(to_range<uint8_t>(col[2], 255));
            out.write(to_range<uint8_t>(col[1], 255));
            out.write(to_range<uint8_t>(col[0], 255));
            break;
            case DDSF_A8R8G8B8:
            out.write(to_range<uint8_t>(col[2], 255));
            out.write(to_range<uint8_t>(col[1], 255));
            out.write(to_range<uint8_t>(col[0], 255));
            out.write(to_range<uint8_t>(col[3], 255));
            break;
            case DDSF_A2R10G10B10: {
                uint32_t word = 0;
                word |= to_range<unsigned>(col[3], 3) << 30;
                word |= to_range<unsigned>(col[0], 1023) << 20;
                word |= to_range<unsigned>(col[1], 1023) << 10;
                word |= to_range<unsigned>(col[2], 1023) << 0;
                out.write(word);
                break;
            }
            case DDSF_A1R5G5B5: {
                uint16_t word = 0;
                word |= to_range<unsigned>(col[3], 1) << 15;
                word |= to_range<unsigned>(col[0], 31) << 10;
                word |= to_range<unsigned>(col[1], 31) << 5;
                word |= to_range<unsigned>(col[2], 31) << 0;
                out.write(word);
                break;
            }
            case DDSF_R8:
            out.write(to_range<uint8_t>(col[0], 255));
            break;
            case DDSF_R16:
            out.write(to_range<uint16_t>(col[0], 65535));
            break;
            case DDSF_G16R16:
            out.write(to_range<uint16_t>(col[0], 65535));
            out.write(to_range<uint16_t>(col[1], 65535));
            break;
            case DDSF_A8R8:
            out.write(to_range<uint8_t>(col[0], 255));
            out.write(to_range<uint8_t>(col[1], 255));
            break;
            case DDSF_A4R4: {
                uint8_t word = 0;
                word |= to_range<unsigned>(col[1], 15) << 4;
                word |= to_range<unsigned>(col[0], 15) << 0;
                out.write(word);
                break;
            }
            case DDSF_A16R16:
            out.write(to_range<uint16_t>(col[0], 65535));
            out.write(to_range<uint16_t>(col[1], 65535));
            break;
            case DDSF_R3G3B2: {
                uint8_t word = 0;
                word |= to_range<unsigned>(col[0], 7) << 5;
                word |= to_range<unsigned>(col[1], 7) << 2;
                word |= to_range<unsigned>(col[2], 3) << 0;
                out.write(word);
                break;
            }
            case DDSF_A4R4G4B4: {
                uint16_t word = 0;
                word |= to_range<unsigned>(col[3], 15) << 12;
                word |= to_range<unsigned>(col[0], 15) << 8;
                word |= to_range<unsigned>(col[1], 15) << 4;
                word |= to_range<unsigned>(col[2], 15) << 0;
                out.write(word);
                break;
            }
            case DDSF_R16F: {
                EXCEPTEX << "Float16 is not implemented." << ENDL;
                break;
            }
            case DDSF_G16R16F: {
                EXCEPTEX << "Float16 is not implemented." << ENDL;
                break;
            }
            case DDSF_R16G16B16A16F: {
                EXCEPTEX << "Float16 is not implemented." << ENDL;
                break;
            }
            case DDSF_R32F: {
                out.write(col[0]);
                break;
            }
            case DDSF_G32R32F: {
                // The ordering of channels here may be wrong, as this format is not documented.
                out.write(col[0]);
                out.write(col[1]);
                break;
            }
            case DDSF_R32G32B32A32F: {
                // The ordering of channels here may be wrong, as this format is not documented.
                out.write(col[0]);
                out.write(col[1]);
                out.write(col[2]);
                out.write(col[3]);
                break;
            }
            default: EXCEPTEX << format << ENDL;
        }
    }

    template<chan_t ch, chan_t ach> void write_image2 (OutFile &out, DDSFormat format, const ImageBase *img_)
    {
        ASSERT(!is_compressed(format));
        const Image<ch,ach> *img = static_cast<const Image<ch,ach>*>(img_);
        for (uimglen_t y=0 ; y<img->height ; ++y) {
            for (uimglen_t x=0 ; x<img->width ; ++x) {
                write_colour(out, format, img->pixel(x,img->height-y-1));
            }
        }
    }

    void initialise_squish_input (const ImageBase *img, squish::u8 *in, squish::u8 *in2, uimglen_t x, uimglen_t y, DDSFormat format)
    {
        for (uimglen_t j=0 ; j<4 ; ++j) {
            for (uimglen_t i=0 ; i<4 ; ++i) {
                switch (format) {
                    case DDSF_BC1:
                    case DDSF_BC2:
                    case DDSF_BC3: {
                        if (x+i >= img->width) continue;
                        if (y+j >= img->height) continue;
                        auto c = static_cast<const Image<3,1>*>(img)->pixel(x+i, img->height-y-j-1);
                        in[4*(j*4+i)+0] = to_range<squish::u8>(c[0], 255);
                        in[4*(j*4+i)+1] = to_range<squish::u8>(c[1], 255);
                        in[4*(j*4+i)+2] = to_range<squish::u8>(c[2], 255);
                        in[4*(j*4+i)+3] = to_range<squish::u8>(c[3], 255);
                    }
                    break;
                    case DDSF_BC4: {
                        if (x+i >= img->width) continue;
                        if (y+j >= img->height) continue;
                        auto c = static_cast<const Image<1,0>*>(img)->pixel(x+i, img->height-y-j-1);
                        in[4*(j*4+i)+0] = 255;
                        in[4*(j*4+i)+1] = 255;
                        in[4*(j*4+i)+2] = 255;
                        in[4*(j*4+i)+3] = to_range<squish::u8>(c[0], 255);
                    }
                    break;
                    case DDSF_BC5: {
                        if (x+i >= img->width) continue;
                        if (y+j >= img->height) continue;
                        auto c = static_cast<const Image<2,0>*>(img)->pixel(x+i, img->height-y-j-1);
                        in[4*(j*4+i)+0] = 255;
                        in[4*(j*4+i)+1] = 255;
                        in[4*(j*4+i)+2] = 255;
                        in[4*(j*4+i)+3] = to_range<squish::u8>(c[0], 255);
                        in2[4*(j*4+i)+0] = 255;
                        in2[4*(j*4+i)+1] = 255;
                        in2[4*(j*4+i)+2] = 255;
                        in2[4*(j*4+i)+3] = to_range<squish::u8>(c[1], 255);
                    }
                    break;
                    default: EXCEPTEX << format << ENDL;
                }
            }
        }

    }

    void write_compressed_image (OutFile &out, DDSFormat format, const ImageBase *img, int squish_flags_)
    {
        ASSERT(is_compressed(format));

        // convert flags to squish enum
        int squish_flags = 0;
        switch (squish_flags_ & 3) {
            case SQUISH_QUALITY_HIGHEST: squish_flags |= squish::kColourIterativeClusterFit; break;
            case SQUISH_QUALITY_HIGH: squish_flags |= squish::kColourClusterFit; break;
            case SQUISH_QUALITY_LOW: squish_flags |= squish::kColourRangeFit; break;
            default: EXCEPT << "Invalid Squish compression flags: " << squish_flags_ << ENDL;
        }
        squish_flags |= (squish_flags_ & SQUISH_METRIC_PERCEPTUAL) ?
                        squish::kColourMetricPerceptual : squish::kColourMetricUniform;
        if (squish_flags_ & SQUISH_WEIGHT_COLOUR_BY_ALPHA) squish_flags |= squish::kWeightColourByAlpha;

        for (uimglen_t y=0 ; y<img->height ; y+=4) {
            for (uimglen_t x=0 ; x<img->width ; x+=4) {
                squish::u8 input[4*4*4] = { 0 };
                squish::u8 input2[4*4*4] = { 0 };
                switch (format) {
                    case DDSF_BC1: {
                        squish::u8 output[8];
                        initialise_squish_input(img, input, input2, x, y, format);
                        squish::Compress(input, output, squish_flags | squish::kDxt1);
                        out.write(output);
                    }
                    break;
                    case DDSF_BC2: {
                        squish::u8 output[16];
                        initialise_squish_input(img, input, input2, x, y, format);
                        squish::Compress(input, output, squish_flags | squish::kDxt3);
                        out.write(output);
                    }
                    break;
                    case DDSF_BC3: {
                        squish::u8 output[16];
                        initialise_squish_input(img, input, input2, x, y, format);
                        squish::Compress(input, output, squish_flags | squish::kDxt5);
                        out.write(output);
                    }
                    break;
                    case DDSF_BC4: {
                        // Convert to RGBA with RGB zero, use DXT5, throw away colour channel
                        squish::u8 output[16];
                        initialise_squish_input(img, input, input2, x, y, format);
                        squish::Compress(input, output, squish_flags | squish::kDxt5);
                        for (unsigned i=0 ; i<8 ; ++i)
                            out.write(output[i]);
                    }
                    break;
                    case DDSF_BC5: {
                        // As BC4, but do it once for each input channel.
                        squish::u8 output[16];
                        squish::u8 output2[16];
                        initialise_squish_input(img, input, input2, x, y, format);
                        squish::Compress(input, output, squish_flags | squish::kDxt5);
                        squish::Compress(input2, output2, squish_flags | squish::kDxt5);
                        for (unsigned i=0 ; i<8 ; ++i)
                            out.write(output2[i]);
                        for (unsigned i=0 ; i<8 ; ++i)
                            out.write(output[i]);
                    }
                    break;
                    default: EXCEPTEX << format << ENDL;
                }
            }
        }
    }

    void write_image (OutFile &out, DDSFormat format, const ImageBase *map, int squish_flags)
    {
        if (is_compressed(format)) {
            write_compressed_image(out, format, static_cast<const Image<3,1>*>(map), squish_flags);
            return;
        }
        // a GCC bug got in the way of
        // (map->hasAlpha()?write_image2<3,1>:write_image2<3,0>)(...);
        switch (map->colourChannels()) {
            case 4:
            write_image2<4,0>(out, format, map);
            break;
            case 3:
            if (map->hasAlpha()) write_image2<3,1>(out, format, map);
            else write_image2<3,0>(out, format, map);
            break;
            case 2:
            if (map->hasAlpha()) write_image2<2,1>(out, format, map);
            else write_image2<2,0>(out, format, map);
            break;
            case 1:
            if (map->hasAlpha()) write_image2<1,1>(out, format, map);
            else write_image2<1,0>(out, format, map);
            break;
            default: EXCEPTEX << map->colourChannels() << ENDL;
        }
    }

    void check_channels_sizes (const std::string &filename, DDSFormat format, const ImageBases &img)
    {
        const ImageBase *top = img[0];

        // sanity checks:
        check_colour(format, top->colourChannels(), top->hasAlpha());

        unsigned expected_width = top->width;
        unsigned expected_height = top->height;
        for (unsigned i=1 ; i<img.size() ; ++i) {
            if (img[i]->colourChannels() != top->colourChannels() || img[i]->hasAlpha() != top->hasAlpha()) {
                EXCEPT << "Couldn't write " << filename << ": All mipmaps must have compatible channels." << ENDL;
            }
            expected_width = expected_width == 1 ? 1 : expected_width/2;
            expected_height = expected_height == 1 ? 1 : expected_height/2;
            if (expected_width != img[i]->width || expected_height != img[i]->height) {
                EXCEPT << "Couldn't write " << filename << ": Mipmap "<<i<<" has the wrong size." << ENDL;
            }
        }
    }

    uint32_t pitch_or_linear_size (DDSFormat format, uimglen_t width, uimglen_t height)
    {
        if (is_compressed(format)) {
            unsigned block_size;
            switch (format) {
                case DDSF_BC1: case DDSF_BC4: block_size = 8;
                break;
                default: block_size = 16;
            }
            unsigned width_blocks = (width + 3)/4;
            if (width_blocks == 0) width_blocks = 1;
            unsigned height_blocks = (height + 3)/4;
            if (height_blocks == 0) height_blocks = 1;
            return width_blocks * height_blocks * block_size;
        } else {
            return (width * bits_per_pixel(format) + 7) / 8;
        }
    }
}

void dds_save (const std::string &filename, DDSFormat format, const DDSFile &content, int squish_flags)
{
    // sanity checks:
    unsigned mipmap_count;
    switch (content.kind) {
        case DDS_SIMPLE:
        mipmap_count = content.simple.size();
        break;
        case DDS_CUBE:
        mipmap_count = content.cube.X.size();
        if (content.cube.x.size() != mipmap_count
            || content.cube.y.size() != mipmap_count
            || content.cube.Y.size() != mipmap_count
            || content.cube.z.size() != mipmap_count
            || content.cube.Z.size() != mipmap_count) {
            EXCEPT << "In \"" << filename << "\", "
                   << "all cube sides must have the same number of mipmaps." << ENDL;
        }
        break;
        case DDS_VOLUME:
        mipmap_count = content.volume.size();
        break;
        default: EXCEPTEX << content.kind << ENDL; // avoid warning
    }
    uimglen_t width;
    uimglen_t height;
    uimglen_t depth;
    ASSERT(mipmap_count > 0u);
    switch (content.kind) {
        case DDS_SIMPLE:
        check_channels_sizes(filename, format, content.simple);
        width = content.simple[0]->width;
        height = content.simple[0]->height;
        depth = 0;
        break;
        case DDS_CUBE:
        check_channels_sizes(filename, format, content.cube.X);
        check_channels_sizes(filename, format, content.cube.x);
        check_channels_sizes(filename, format, content.cube.Y);
        check_channels_sizes(filename, format, content.cube.y);
        check_channels_sizes(filename, format, content.cube.Z);
        check_channels_sizes(filename, format, content.cube.z);
        width = content.cube.X[0]->width;
        height = content.cube.Y[0]->height;
        depth = 0;
        if (content.cube.x[0]->width != width
            || content.cube.Y[0]->width != width
            || content.cube.y[0]->width != width
            || content.cube.Z[0]->width != width
            || content.cube.z[0]->width != width
            || content.cube.x[0]->height != height
            || content.cube.Y[0]->height != height
            || content.cube.y[0]->height != height
            || content.cube.Z[0]->height != height
            || content.cube.z[0]->height != height)
            EXCEPT << "In \"" << filename << "\", cube faces must be square and the same size." << ENDL;
        break;
        case DDS_VOLUME: {
            depth = content.volume[0].size();
            ASSERT(depth > 0u);
            width = content.volume[0][0]->width;
            height = content.volume[0][0]->height;
            for (unsigned mip=0 ; mip<mipmap_count ; ++mip) {
                uimglen_t mip_width = content.volume[mip][0]->width;
                uimglen_t mip_height = content.volume[mip][0]->height;
                uimglen_t mip_depth = content.volume[mip].size();
                uimglen_t exp_width = width >> mip > 0 ? width >> mip : 1;
                uimglen_t exp_height = height >> mip > 0 ? height >> mip : 1;
                uimglen_t exp_depth = depth >> mip > 0 ? depth >> mip : 1;
                if (mip_width != exp_width || mip_height != exp_height || mip_depth != exp_depth) {
                    EXCEPT << "Couldn't write \""<<filename<<"\" as mipmap "<<mip<<" had the wrong size ("
                           << mip_width << ", " << mip_height << ", " << mip_depth << ") expected ("
                           << exp_width << ", " << exp_height << ", " << exp_depth << ")." << ENDL;
                }
                for (uimglen_t z=0 ; z<mip_depth ; ++z) {
                    const ImageBase *layer = content.volume[mip][z];
                    check_colour(format, layer->colourChannels(), layer->hasAlpha());
                    if (layer->width != mip_width || layer->height != mip_height) {
                        EXCEPT << "Couldn't write \""<<filename<<"\" as mipmap "<<mip<<" had a layer with the wrong size ("
                               << layer->width << ", " << layer->width << ") expected ("
                               << exp_width << ", " << exp_height << ")." << ENDL;
                    }
                }
            }
        }
        break;
        default: EXCEPTEX << content.kind << ENDL; // avoid warning
    }

    OutFile out(filename);

    // Filetype magic
    out.write(FOURCC('D', 'D', 'S', ' '));

    // DDS_HEADER
    uint32_t flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    if (mipmap_count > 1) flags |= DDSD_MIPMAPCOUNT;
    flags |= is_compressed(format) ? DDSD_LINEARSIZE :  DDSD_PITCH;
    if (content.kind == DDS_VOLUME) flags |= DDSD_DEPTH;
    out.write(uint32_t(124));
    out.write(flags);
    out.write(uint32_t(height));
    out.write(uint32_t(width));
    out.write(pitch_or_linear_size(format, width, height));
    out.write(uint32_t(depth)); // DDSD_DEPTH
    out.write(uint32_t(mipmap_count)); // DDSD_MIPMAPCOUNT
    for (int i=0 ; i<11 ; ++i) out.write(uint32_t(0)); //unused
    output_pixelformat(out, format);
    uint32_t caps = DDSCAPS_TEXTURE;
    if (mipmap_count > 1) caps |= DDSCAPS_MIPMAP;
    if (mipmap_count > 1 || content.kind == DDS_CUBE) caps |= DDSCAPS_COMPLEX;
    out.write(caps);
    uint32_t caps2 = 0;
    if (content.kind == DDS_CUBE) {
        caps2 |= DDSCAPS2_CUBEMAP;
        caps2 |= DDSCAPS2_CUBEMAP_POSITIVEX;
        caps2 |= DDSCAPS2_CUBEMAP_NEGATIVEX;
        caps2 |= DDSCAPS2_CUBEMAP_POSITIVEY;
        caps2 |= DDSCAPS2_CUBEMAP_NEGATIVEY;
        caps2 |= DDSCAPS2_CUBEMAP_POSITIVEZ;
        caps2 |= DDSCAPS2_CUBEMAP_NEGATIVEZ;
    } else if (content.kind == DDS_VOLUME) {
        caps2 |= DDSCAPS2_VOLUME;
    }
    out.write(caps2);
    out.write(uint32_t(0)); // caps3
    out.write(uint32_t(0)); // caps4
    out.write(uint32_t(0)); // unused

    // DDS_HEADER_DX10
    if (false) {
        uint32_t dx10_format = 0;
        uint32_t resource_dimension = 0;
        uint32_t misc_flag = 0;
        uint32_t array_size = 0;
        uint32_t misc_flags2 = 0;
        out.write(dx10_format);
        out.write(resource_dimension);
        out.write(misc_flag);
        out.write(array_size);
        out.write(misc_flags2);
    }

    switch (content.kind) {
        case DDS_SIMPLE: {
            for (unsigned i=0 ; i<mipmap_count ; ++i) {
                write_image(out, format, content.simple[i], squish_flags);
            }
        } break;
        case DDS_CUBE: {
            for (unsigned i=0 ; i<mipmap_count ; ++i)
                write_image(out, format, content.cube.X[i], squish_flags);
            for (unsigned i=0 ; i<mipmap_count ; ++i)
                write_image(out, format, content.cube.x[i], squish_flags);
            for (unsigned i=0 ; i<mipmap_count ; ++i)
                write_image(out, format, content.cube.Y[i], squish_flags);
            for (unsigned i=0 ; i<mipmap_count ; ++i)
                write_image(out, format, content.cube.y[i], squish_flags);
            for (unsigned i=0 ; i<mipmap_count ; ++i)
                write_image(out, format, content.cube.Z[i], squish_flags);
            for (unsigned i=0 ; i<mipmap_count ; ++i)
                write_image(out, format, content.cube.z[i], squish_flags);
        } break;
        case DDS_VOLUME: {
            for (unsigned i=0 ; i<mipmap_count ; ++i)
                for (unsigned z=0 ; z<content.volume[i].size() ; ++z)
                    write_image(out, format, content.volume[i][z], squish_flags);
        } break;
    }
}

namespace {
    // assumes ch is the number of non-zero rgb masks and ach is 1 if a_mask is non-zero
    template<chan_t ch, chan_t ach> Image<ch,ach> *read_rgb_image(InFile &in, uimglen_t width, uimglen_t height,
                                                                  unsigned bytes,
                                                                  uint32_t r_mask, uint32_t g_mask,
                                                                  uint32_t b_mask, uint32_t a_mask)
    {
        Image<ch,ach> *nu = new Image<ch,ach>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                uint32_t word = 0;
                // read little endian
                for (unsigned i=0 ; i<bytes ; ++i) {
                    word |= in.read<unsigned char>() << (i*8);
                }
                chan_t i = 0;
                if (r_mask != 0) nu->pixel(x, height-y-1)[i++] = float(word & r_mask)/r_mask;
                if (g_mask != 0) nu->pixel(x, height-y-1)[i++] = float(word & g_mask)/g_mask;
                if (b_mask != 0) nu->pixel(x, height-y-1)[i++] = float(word & b_mask)/b_mask;
                if (a_mask != 0) nu->pixel(x, height-y-1)[i++] = float(word & a_mask)/a_mask;
            }
        }
        return nu;
    }

    Colour<3,1> decode_r5g6b5 (uint16_t col)
    {
        Colour<3,1> r;
        r[0] = float(col & 0xf800) / 0xf800;
        r[1] = float(col & 0x07e0) / 0x07e0;
        r[2] = float(col & 0x001f) / 0x001f;
        r[3] = 1.0;
        return r;
    }

    void draw_compressed_block (Image<3,1> *nu, uimglen_t x, uimglen_t y,
                                uint16_t col1, uint16_t col2, uint32_t lu, bool use_alpha_mask)
    {
        Colour<3,1> palette[4];
        if (col1 > col2 || !use_alpha_mask) {
            palette[0] = decode_r5g6b5(col1);
            palette[1] = decode_r5g6b5(col2);
            for (chan_t c=0 ; c<3 ; ++c) {
                palette[2][c] = (2*palette[0][c] + palette[1][c])/3;
                palette[3][c] = (palette[0][c] + 2*palette[1][c])/3;
            }
            palette[2][3] = 1;
            palette[3][3] = 1;
        } else {
            palette[0] = decode_r5g6b5(col1);
            palette[1] = decode_r5g6b5(col2);
            for (chan_t c=0 ; c<3 ; ++c) {
                palette[2][c] = (palette[0][c] + palette[1][c])/2;
            }
            palette[2][3] = 1;
            palette[3][0] = 0;
            palette[3][1] = 0;
            palette[3][2] = 0;
            palette[3][3] = 0;
        }
        for (uimglen_t yoff=0 ; yoff<4 ; ++yoff) {
            uimglen_t y2 = y + yoff;
            if (y2 >= nu->height) break;
            for (uimglen_t xoff=0 ; xoff<4 ; ++xoff) {
                uimglen_t x2 = x + xoff;
                if (x2 >= nu->width) break;
                int p = (lu >> (yoff*4 + xoff)*2) & 0x3;
                nu->pixel(x2, nu->height-y2-1) = palette[p];
            }
        }
    }

    template<chan_t ch, chan_t ach, chan_t into>
    void draw_compressed_alpha_block (Image<ch,ach> *nu, uimglen_t x, uimglen_t y, uint64_t alpha_blob)
    {
        float a_palette[8];
        a_palette[0] = ((alpha_blob >> 0) & 0xff) / 255.0f;
        a_palette[1] = ((alpha_blob >> 8) & 0xff) / 255.0f;
        auto alpha_lu = alpha_blob >> 16;
        if (a_palette[0] > a_palette[1]) {
            a_palette[2] = (6*a_palette[0] + 1*a_palette[1])/7;
            a_palette[3] = (5*a_palette[0] + 2*a_palette[1])/7;
            a_palette[4] = (4*a_palette[0] + 3*a_palette[1])/7;
            a_palette[5] = (3*a_palette[0] + 4*a_palette[1])/7;
            a_palette[6] = (2*a_palette[0] + 5*a_palette[1])/7;
            a_palette[7] = (1*a_palette[0] + 6*a_palette[1])/7;
        } else {
            a_palette[2] = (4*a_palette[0] + 1*a_palette[1])/5;
            a_palette[3] = (3*a_palette[0] + 2*a_palette[1])/5;
            a_palette[4] = (2*a_palette[0] + 3*a_palette[1])/5;
            a_palette[5] = (1*a_palette[0] + 4*a_palette[1])/5;
            a_palette[6] = 0;
            a_palette[7] = 1;
        }
        for (uimglen_t yoff=0 ; yoff<4 ; ++yoff) {
            uimglen_t y2 = y + yoff;
            if (y2 >= nu->height) break;
            for (uimglen_t xoff=0 ; xoff<4 ; ++xoff) {
                uimglen_t x2 = x + xoff;
                if (x2 >= nu->width) break;
                int p = (alpha_lu >> (yoff*4 + xoff)*3) & 0x7;
                nu->pixel(x2, nu->height-y2-1)[into] = a_palette[p];
            }
        }
    }

    ImageBase *read_compressed_image (InFile &in, uimglen_t width, uimglen_t height, uint32_t pf_fourcc)
    {
        DDSFormat codec;
        
        switch (pf_fourcc) {
            case FOURCC('D', 'X', 'T', '1'): codec = DDSF_BC1; break;
            case FOURCC('D', 'X', 'T', '2'):
            case FOURCC('D', 'X', 'T', '3'): codec = DDSF_BC2; break;
            case FOURCC('D', 'X', 'T', '4'):
            case FOURCC('D', 'X', 'T', '5'): codec = DDSF_BC3; break;
            case FOURCC('A', 'T', 'I', '1'): codec = DDSF_BC4; break;
            case FOURCC('A', 'T', 'I', '2'): codec = DDSF_BC5; break;
            case 0x6F: codec = DDSF_R16F; break;
            case 0x70: codec = DDSF_G16R16F; break;
            case 0x71: codec = DDSF_R16G16B16A16F; break;
            case 0x72: codec = DDSF_R32F; break;
            case 0x73: codec = DDSF_G32R32F; break;
            case 0x74: codec = DDSF_R32G32B32A32F; break;
            default:
            EXCEPT << "DDS file \""<<in.filename<<"\" has unrecognised fourcc:" << std::hex << pf_fourcc << ENDL;
        }
        switch (codec) {
            case DDSF_BC1: {
                auto *nu = new Image<3,1>(width, height);
                for (uimglen_t y=0 ; y<height ; y+=4) {
                    for (uimglen_t x=0 ; x<width ; x+=4) {
                        auto col1 = in.read<uint16_t>();
                        auto col2 = in.read<uint16_t>();
                        auto lu = in.read<uint32_t>();
                        draw_compressed_block(nu, x, y, col1, col2, lu, true);
                    }
                }
                return nu;
            }
            case DDSF_BC2: {
                auto *nu = new Image<3,1>(width, height);
                for (uimglen_t y=0 ; y<height ; y+=4) {
                    for (uimglen_t x=0 ; x<width ; x+=4) {
                        auto alpha = in.read<uint64_t>();
                        auto col1 = in.read<uint16_t>();
                        auto col2 = in.read<uint16_t>();
                        auto lu = in.read<uint32_t>();
                        draw_compressed_block(nu, x, y, col1, col2, lu, false);
                        for (uimglen_t yoff=0 ; yoff<4 ; ++yoff) {
                            uimglen_t y2 = y + yoff;
                            if (y2 >= nu->height) break;
                            for (uimglen_t xoff=0 ; xoff<4 ; ++xoff) {
                                uimglen_t x2 = x + xoff;
                                if (x2 >= nu->width) break;
                                int a = (alpha >> (yoff*4 + xoff)*4) & 0xf;
                                nu->pixel(x2, nu->height-y2-1)[3] = (a * 16)/255.0;
                            }
                        }
                    }
                }
                return nu;
            }
            case DDSF_BC3: {
                auto *nu = new Image<3,1>(width, height);
                for (uimglen_t y=0 ; y<height ; y+=4) {
                    for (uimglen_t x=0 ; x<width ; x+=4) {
                        auto alpha_blob = in.read<uint64_t>();
                        auto col1 = in.read<uint16_t>();
                        auto col2 = in.read<uint16_t>();
                        auto lu = in.read<uint32_t>();
                        draw_compressed_block(nu, x, y, col1, col2, lu, false);
                        draw_compressed_alpha_block<3,1,3>(nu, x, y, alpha_blob);
                    }
                }
                return nu;
            }
            case DDSF_BC4: {
                auto *nu = new Image<1,0>(width, height);
                for (uimglen_t y=0 ; y<height ; y+=4) {
                    for (uimglen_t x=0 ; x<width ; x+=4) {
                        auto alpha_blob = in.read<uint64_t>();
                        draw_compressed_alpha_block<1,0,0>(nu, x, y, alpha_blob);
                    }
                }
                return nu;
            }
            case DDSF_BC5: {
                auto *nu = new Image<2,0>(width, height);
                for (uimglen_t y=0 ; y<height ; y+=4) {
                    for (uimglen_t x=0 ; x<width ; x+=4) {
                        auto alpha_blob1 = in.read<uint64_t>();
                        auto alpha_blob2 = in.read<uint64_t>();
                        draw_compressed_alpha_block<2,0,1>(nu, x, y, alpha_blob1);
                        draw_compressed_alpha_block<2,0,0>(nu, x, y, alpha_blob2);
                    }
                }
                return nu;
            }
            case DDSF_R16F: {
                auto *nu = new Image<1,0>(width, height);
                EXCEPTEX << "Float16 not implemented." << ENDL;
                return nu;
            }
            case DDSF_G16R16F: {
                auto *nu = new Image<2,0>(width, height);
                EXCEPTEX << "Float16 not implemented." << ENDL;
                return nu;
            }
            case DDSF_R16G16B16A16F: {
                auto *nu = new Image<3,1>(width, height);
                EXCEPTEX << "Float16 not implemented." << ENDL;
                return nu;
            }
            case DDSF_R32F: {
                auto *nu = new Image<1,0>(width, height);
                for (uimglen_t y=0 ; y<height ; ++y) {
                    for (uimglen_t x=0 ; x<width ; ++x) {
                        nu->pixel(x, y)[0] = in.read<float>();
                    }
                }
                return nu;
            }
            case DDSF_G32R32F: {
                auto *nu = new Image<2,0>(width, height);
                for (uimglen_t y=0 ; y<height ; ++y) {
                    for (uimglen_t x=0 ; x<width ; ++x) {
                        // The ordering of channels may be wrong here, as this format is not
                        // documented.
                        nu->pixel(x, y)[1] = in.read<float>();
                        nu->pixel(x, y)[0] = in.read<float>();
                    }
                }
                return nu;
            }
            case DDSF_R32G32B32A32F: {
                auto *nu = new Image<3,1>(width, height);
                for (uimglen_t y=0 ; y<height ; ++y) {
                    for (uimglen_t x=0 ; x<width ; ++x) {
                        // The ordering of channels may be wrong here, as this format is not
                        // documented.
                        nu->pixel(x, y)[0] = in.read<float>();
                        nu->pixel(x, y)[1] = in.read<float>();
                        nu->pixel(x, y)[2] = in.read<float>();
                        nu->pixel(x, y)[3] = in.read<float>();
                    }
                }
                return nu;
            }
            default: EXCEPTEX << codec << ENDL;
        }
    }

    ImageBase *read_mipmap (InFile &in, uimglen_t width, uimglen_t height, uint32_t pf_rgb_bitcount,
                            uint32_t pf_r_mask, uint32_t pf_g_mask, uint32_t pf_b_mask, uint32_t pf_a_mask,
                            uint32_t pf_flags, uint32_t pf_fourcc)
    {
        chan_t ch=0, ach=0;
        ImageBase *nu = NULL;
        if (pf_flags & DDPF_RGB) {
            if ((pf_r_mask | pf_g_mask | pf_b_mask) == 0) {
                EXCEPT << "DDS file \""<<in.filename<<"\" has all RGB masks set to 0." << ENDL;
            }
            switch (pf_rgb_bitcount) {
                case 8: case 16: case 24: case 32: break;
                default:
                EXCEPT << "DDS file \""<<in.filename<<"\" has unsupported RGB bitcount: " << pf_rgb_bitcount << ENDL;
            }
            unsigned bytes = pf_rgb_bitcount / 8;
            if (!(pf_flags & DDPF_ALPHAPIXELS)) pf_a_mask = 0;
            if (pf_r_mask != 0) ch++;
            if (pf_g_mask != 0) ch++;
            if (pf_b_mask != 0) ch++;
            if (pf_a_mask) ach = 1;
            switch (ch) {
                case 1: 
                if (ach == 1) {
                    nu = read_rgb_image<1,1>(in, width, height, bytes, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask);
                } else {
                    nu = read_rgb_image<1,0>(in, width, height, bytes, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask);
                }
                break;
                case 2: 
                if (ach == 1) {
                    nu = read_rgb_image<2,1>(in, width, height, bytes, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask);
                } else {
                    nu = read_rgb_image<2,0>(in, width, height, bytes, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask);
                }
                break;
                case 3: 
                if (ach == 1) {
                    nu = read_rgb_image<3,1>(in, width, height, bytes, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask);
                } else {
                    nu = read_rgb_image<3,0>(in, width, height, bytes, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask);
                }
                break;
                case 4: 
                nu = read_rgb_image<4,0>(in, width, height, bytes, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask);
                break;
            }
        } else if (pf_flags & DDPF_FOURCC) {
            nu = read_compressed_image(in, width, height, pf_fourcc);
        } else {
            EXCEPT << "DDS file \""<<in.filename<<"\" has neither fourcc nor RGB pixel format." << ENDL;
        }
        return nu;
    }

    ImageBases read_mipmaps (InFile &in, uimglen_t width, uimglen_t height, uint32_t pf_rgb_bitcount,
                             uint32_t r_mask, uint32_t g_mask, uint32_t b_mask, uint32_t a_mask,
                             uint32_t pf_flags, uint32_t pf_fourcc, unsigned mipmap_count)
    {
        ImageBases mips;
        for (unsigned i=0 ; i<mipmap_count ; ++i) {
            mips.push_back(read_mipmap(in, width, height, pf_rgb_bitcount, r_mask, g_mask, b_mask, a_mask, pf_flags, pf_fourcc));
            width = width == 1 ? 1 : width / 2;
            height = height == 1 ? 1 : height / 2;
        }
        return mips;
    }
}

DDSFile dds_open (const std::string &filename)
{
    DDSFile file;
    InFile in(filename);

    uint32_t magic = in.read<uint32_t>();
    if (magic != FOURCC('D', 'D', 'S', ' ')) {
        EXCEPT << "Not a DDS file: \"" << filename << "\"" << ENDL;
    }

    uint32_t sz = in.read<uint32_t>();
    if (sz != 124) EXCEPT << "DDS header of \""<<filename<<"\" had wrong size: " << sz << ENDL;
    uint32_t flags = in.read<uint32_t>();
    (void) flags; // they are too frequently wrong to bother reading
    uint32_t height = in.read<uint32_t>();
    uint32_t width = in.read<uint32_t>();
    in.read<uint32_t>(); // pitch_or_linear_size: can't be relied upon
    uint32_t depth = in.read<uint32_t>();
    uint32_t mipmap_count = in.read<uint32_t>();
    // don't rely on DDSD_MIPMAPCOUNT flag being set
    if (mipmap_count == 0) mipmap_count = 1;
    for (int i=0 ; i<11 ; ++i) in.read<uint32_t>(); //unused

    uint32_t pf_sz = in.read<uint32_t>();
    if (pf_sz != 32) EXCEPT << "DDS PixelFormat header of \""<<filename<<"\" had wrong size: " << pf_sz << ENDL;
    uint32_t pf_flags = in.read<uint32_t>();
    uint32_t pf_fourcc = in.read<uint32_t>();
    uint32_t pf_rgb_bitcount = in.read<uint32_t>();
    uint32_t pf_r_mask = in.read<uint32_t>();
    uint32_t pf_g_mask = in.read<uint32_t>();
    uint32_t pf_b_mask = in.read<uint32_t>();
    uint32_t pf_a_mask = in.read<uint32_t>();


    in.read<uint32_t>(); // caps: can't be relied upon
    uint32_t caps2 = in.read<uint32_t>(); // cubemap, volume map
    in.read<uint32_t>(); // caps3
    in.read<uint32_t>(); // caps4
    in.read<uint32_t>(); // unused

    if ((pf_flags & DDPF_FOURCC) && pf_fourcc==FOURCC('D', 'X', '1', '0')) {
        uint32_t dx10_format = in.read<uint32_t>();
        uint32_t resource_dimension = in.read<uint32_t>();
        uint32_t misc_flag = in.read<uint32_t>();
        uint32_t array_size = in.read<uint32_t>();
        uint32_t misc_flags2 = in.read<uint32_t>();
        (void) dx10_format;
        (void) resource_dimension;
        (void) misc_flag;
        (void) array_size;
        (void) misc_flags2;
        EXCEPT << "DDS DX10 header not yet supported in \"" << filename << "\"" << ENDL;
    }
    if (caps2 & DDSCAPS2_CUBEMAP) {
        file.kind = DDS_CUBE;
    } else if (caps2 & DDSCAPS2_VOLUME) {
        file.kind = DDS_VOLUME;
    } else {
        file.kind = DDS_SIMPLE;
    }

    ImageBases mips;
    switch (file.kind) {
        case DDS_SIMPLE:
        file.simple = read_mipmaps(in, width, height, pf_rgb_bitcount, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask,
                                   pf_flags, pf_fourcc, mipmap_count);
        break;
        case DDS_CUBE:
        file.cube.X = read_mipmaps(in, width, height, pf_rgb_bitcount, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask,
                                   pf_flags, pf_fourcc, mipmap_count);
        file.cube.x = read_mipmaps(in, width, height, pf_rgb_bitcount, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask,
                                   pf_flags, pf_fourcc, mipmap_count);
        file.cube.Y = read_mipmaps(in, width, height, pf_rgb_bitcount, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask,
                                   pf_flags, pf_fourcc, mipmap_count);
        file.cube.y = read_mipmaps(in, width, height, pf_rgb_bitcount, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask,
                                   pf_flags, pf_fourcc, mipmap_count);
        file.cube.Z = read_mipmaps(in, width, height, pf_rgb_bitcount, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask,
                                   pf_flags, pf_fourcc, mipmap_count);
        file.cube.z = read_mipmaps(in, width, height, pf_rgb_bitcount, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask,
                                   pf_flags, pf_fourcc, mipmap_count);
        break;
        case DDS_VOLUME: {
            file.volume.resize(mipmap_count);
            for (unsigned i=0 ; i<mipmap_count ; ++i) {
                ImageBases &layers = file.volume[i];
                for (unsigned j=0 ; j<depth ; ++j) {
                    layers.push_back(read_mipmap(in, width, height, pf_rgb_bitcount, pf_r_mask, pf_g_mask, pf_b_mask, pf_a_mask,
                                                 pf_flags, pf_fourcc));
                }
                width = width == 1 ? 1 : width / 2;
                height = height == 1 ? 1 : height / 2;
                depth = depth == 1 ? 1 : depth / 2;
            }
        } break;
    }

    return file;
}
