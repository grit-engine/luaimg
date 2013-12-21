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

#include "dds.h"

#include <io_util.h>

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


void dds_save_simple (const std::string &filename, DDSFormat format, const DDSMipmaps &img)
{
    ASSERT(img.levels > 0u);
    const ImageBase *top = img.maps[0];
    (void) format;
    std::ofstream out;
    io_util_open(filename, out);

    // Filetype magic
    io_util_write(filename, out, 'D');
    io_util_write(filename, out, 'D');
    io_util_write(filename, out, 'S');
    io_util_write(filename, out, ' ');

    // DDS_HEADER
    uint32_t flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT
                   | DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE;
    uint32_t height = top->height;
    uint32_t width = top->width;
    uint32_t pitch_or_linear_size = 0;
    uint32_t depth = 0;
    uint32_t mipmap_count = 0;
    io_util_write(filename, out, uint32_t(124));
    io_util_write(filename, out, flags);
    io_util_write(filename, out, height);
    io_util_write(filename, out, width);
    io_util_write(filename, out, pitch_or_linear_size);
    io_util_write(filename, out, depth);
    io_util_write(filename, out, mipmap_count);
    for (int i=0 ; i<11 ; ++i) io_util_write(filename, out, uint32_t(0));
    {
        // DDS_HEADER.PIXELFORMAT
        uint32_t flags = 0;
        uint32_t fourcc = 0;
        uint32_t rgb_bit_count = 0;
        uint32_t r_mask = 0;
        uint32_t g_mask = 0;
        uint32_t b_mask = 0;
        uint32_t a_mask = 0;
        io_util_write(filename, out, uint32_t(124));
        io_util_write(filename, out, flags);
        io_util_write(filename, out, fourcc);
        io_util_write(filename, out, rgb_bit_count);
        io_util_write(filename, out, r_mask);
        io_util_write(filename, out, g_mask);
        io_util_write(filename, out, b_mask);
        io_util_write(filename, out, a_mask);
    }
    uint32_t caps = 0;
    uint32_t caps2 = 0;
    uint32_t caps3 = 0;
    uint32_t caps4 = 0;
    io_util_write(filename, out, caps);
    io_util_write(filename, out, caps2);
    io_util_write(filename, out, caps3);
    io_util_write(filename, out, caps4);
    io_util_write(filename, out, uint32_t(0));

    // DDS_HEADER_DX10
    uint32_t dx10_format = 0;
    uint32_t resource_dimension = 0;
    uint32_t misc_flag = 0;
    uint32_t array_size = 0;
    uint32_t misc_flags2 = 0;
    io_util_write(filename, out, dx10_format);
    io_util_write(filename, out, resource_dimension);
    io_util_write(filename, out, misc_flag);
    io_util_write(filename, out, array_size);
    io_util_write(filename, out, misc_flags2);
    
    out.close();
}
