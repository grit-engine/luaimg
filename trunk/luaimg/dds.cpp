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
    else if (str == "DXT1") return DDSF_DXT1;
    else if (str == "DXT3") return DDSF_DXT3;
    else if (str == "DXT5") return DDSF_DXT5;
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
        case DDSF_DXT1: return "DXT1";
        case DDSF_DXT3: return "DXT3";
        case DDSF_DXT5: return "DXT5";
        default: EXCEPTEX << format << ENDL;
    }
}

namespace {

    void check_colour (DDSFormat format, chan_t ch, bool alpha)
    {
        switch (format) {
            case DDSF_A2R10G10B10:
            case DDSF_A1R5G5B5:
            case DDSF_A8R8G8B8:
            case DDSF_A4R4G4B4:
            case DDSF_DXT1:
            case DDSF_DXT3:
            case DDSF_DXT5:
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

            case DDSF_DXT1:
            return 4;

            case DDSF_DXT3:
            case DDSF_DXT5:
            case DDSF_R8:
            case DDSF_R3G3B2:
            case DDSF_A4R4:
            return 8;

            case DDSF_R16:
            case DDSF_A8R8:
            case DDSF_R5G6B5:
            case DDSF_A1R5G5B5:
            case DDSF_A4R4G4B4:
            return 16;

            case DDSF_R8G8B8:
            return 24;

            case DDSF_A16R16:
            case DDSF_A8R8G8B8:
            case DDSF_A2R10G10B10:
            case DDSF_G16R16:
            return 32;

            default: EXCEPTEX << format << ENDL;
        }
    }

    bool is_compressed (DDSFormat format)
    {
        switch (format) {
            case DDSF_DXT1:
            case DDSF_DXT3:
            case DDSF_DXT5:
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
            r_mask = 0x00004800;
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
            case DDSF_DXT1:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = FOURCC('D','X','T','1');
            break;
            case DDSF_DXT3:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = FOURCC('D','X','T','3');
            break;
            case DDSF_DXT5:
            rgb_bitcount = 0;
            flags = DDPF_FOURCC;
            fourcc = FOURCC('D','X','T','5');
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

    void write_compressed_image (OutFile &out, DDSFormat format, const Image<3,1> *img, int dxt_flags)
    {
        ASSERT(is_compressed(format));

        // convert flags to squish enum
        int squish_flags = 0;
        switch (dxt_flags & 3) {
            case DXT_QUALITY_HIGHEST: squish_flags |= squish::kColourIterativeClusterFit; break;
            case DXT_QUALITY_HIGH: squish_flags |= squish::kColourClusterFit; break;
            case DXT_QUALITY_LOW: squish_flags |= squish::kColourRangeFit; break;
            default: EXCEPT << "Invalid DXT compression flags: " << dxt_flags << ENDL;
        }
        squish_flags |= (dxt_flags & DXT_METRIC_PERCEPTUAL) ?
                        squish::kColourMetricPerceptual : squish::kColourMetricUniform;
        if (dxt_flags & DXT_WEIGHT_COLOUR_BY_ALPHA) squish_flags |= squish::kWeightColourByAlpha;

        for (uimglen_t y=0 ; y<img->height ; y+=4) {
            for (uimglen_t x=0 ; x<img->width ; x+=4) {
                squish::u8 input[4*4*4] = { 0 };
                for (uimglen_t j=0 ; j<4 ; ++j) {
                    for (uimglen_t i=0 ; i<4 ; ++i) {
                        if (x+i >= img->width) continue;
                        if (y+j >= img->height) continue;
                        Colour<3,1> c = img->pixel(x+i, img->height-y-j-1);
                        input[4*(j*4+i)+0] = to_range<squish::u8>(c[0], 255);
                        input[4*(j*4+i)+1] = to_range<squish::u8>(c[1], 255);
                        input[4*(j*4+i)+2] = to_range<squish::u8>(c[2], 255);
                        input[4*(j*4+i)+3] = to_range<squish::u8>(c[3], 255);
                    }
                }
                switch (format) {
                    case DDSF_DXT1: {
                        squish::u8 output[8];
                        squish::Compress(input, output, squish_flags | squish::kDxt1);
                        out.write(output);
                    }
                    break;
                    case DDSF_DXT3: {
                        squish::u8 output[16];
                        squish::Compress(input, output, squish_flags | squish::kDxt3);
                        out.write(output);
                    }
                    break;
                    case DDSF_DXT5: {
                        squish::u8 output[16];
                        squish::Compress(input, output, squish_flags | squish::kDxt5);
                        out.write(output);
                    }
                    break;
                    default: EXCEPTEX << format << ENDL;
                }
            }
        }
    }

    void write_image (OutFile &out, DDSFormat format, const ImageBase *map)
    {
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

    void check_mipmaps (const std::string &filename, DDSFormat format, const ImageBases &img)
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
                case DDSF_DXT1:
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

void dds_save_simple (const std::string &filename, DDSFormat format, const ImageBases &img, int dxt_flags)
{
    ASSERT(img.size() > 0u);
    const ImageBase *top = img[0];

    // sanity checks:
    check_mipmaps(filename, format, img);

    OutFile out(filename);

    // Filetype magic
    out.write(FOURCC('D', 'D', 'S', ' '));

    // DDS_HEADER
    uint32_t flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    if (img.size() > 1) flags |= DDSD_MIPMAPCOUNT;
    flags |= is_compressed(format) ? DDSD_LINEARSIZE :  DDSD_PITCH;
    out.write(uint32_t(124));
    out.write(flags);
    out.write(uint32_t(top->height));
    out.write(uint32_t(top->width));
    out.write(pitch_or_linear_size(format, top->width, top->height));
    out.write(uint32_t(0)); // DDS_DEPTH
    out.write(uint32_t(img.size())); // DDSD_MIPMAPCOUNT
    for (int i=0 ; i<11 ; ++i) out.write(uint32_t(0)); //unused
    output_pixelformat(out, format);
    uint32_t caps = DDSCAPS_TEXTURE;
    if (img.size() > 1) caps |= DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
    uint32_t caps2 = 0; // used for cubes
    out.write(caps);
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

    if (is_compressed(format)) {
        for (unsigned i=0 ; i<img.size() ; ++i) {
            write_compressed_image(out, format, static_cast<const Image<3,1>*>(img[i]), dxt_flags);
        }
    } else {
        for (unsigned i=0 ; i<img.size() ; ++i) {
            write_image(out, format, img[i]);
        }
    }
    
}

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
    } else {
        ASSERT(depth == 0);
        ASSERT((flags & DDSD_DEPTH) == 0);
        ASSERT(caps2 == 0); // fix when using cubemap or volume map
    }
    
    file.kind = DDS_SIMPLE;
    for (unsigned i=0 ; i<mipmap_count ; ++i) {
        chan_t ch=0, ach=0;
        ImageBase *nu = NULL;
        if (pf_flags & DDPF_RGB) {
            if ((pf_r_mask | pf_g_mask | pf_b_mask) == 0) {
                EXCEPT << "DDS file \""<<filename<<"\" has all RGB masks set to 0." << ENDL;
            }
            switch (pf_rgb_bitcount) {
                case 8: case 16: case 24: case 32: break;
                default:
                EXCEPT << "DDS file \""<<filename<<"\" has unsupported RGB bitcount: " << pf_rgb_bitcount << ENDL;
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
            EXCEPT << "DDS file \""<<filename<<"\" has neither fourcc nor RGB pixel format." << ENDL;
        }
        file.simple.push_back(nu);

        width = width == 1 ? 1 : width / 2;
        height = height == 1 ? 1 : height / 2;
    }

    return file;
}

Image<3,1> *read_compressed_image(InFile &in, uimglen_t width, uimglen_t height, uint32_t pf_fourcc)
{
    int dxt;
    switch (pf_fourcc) {
        case FOURCC('D', 'X', 'T', '1'): dxt = 1; break;
        case FOURCC('D', 'X', 'T', '2'):
        case FOURCC('D', 'X', 'T', '3'): dxt = 3; break;
        case FOURCC('D', 'X', 'T', '4'):
        case FOURCC('D', 'X', 'T', '5'): dxt = 5; break;
        default:
        EXCEPT << "DDS file \""<<filename<<"\" has unrecognised fourcc:" << pf_fourcc << ENDL;
    }
}
