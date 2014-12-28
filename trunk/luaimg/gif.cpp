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


#include <cstdlib>
#include <cstdint>

#include <memory>

extern "C" {
    #include <gif_lib.h>
}

#include <io_util.h>
#include <console.h>
#include <exception.h>

#include "gif.h"
#include "image.h"


float clamp (float x)
{
    if (x < 0) return 0;
    if (x > 1) return 1;
    return x;
}

struct ColorMapPtr {
    ColorMapObject * const palette;
    ColorMapPtr (unsigned sz)
      : palette(GifMakeMapObject(sz, nullptr))
    {
        if (palette == nullptr)
            EXCEPT << "Libgif unable to make palette" << ENDL;
    }
    ~ColorMapPtr (void)
    {
        GifFreeMapObject(palette);
    }
};

struct ScopedFile {
    GifFileType * const file;
    const std::string filename;
    int openErr;
    ScopedFile (GifFileType *file, const std::string filename)
      : file(file), filename(filename)
    { }
    NORETURN1 std::string throwErr (const std::string &msg) NORETURN2
    {
        EXCEPT << filename << ": " << GifErrorString(file->Error) << " while " << msg << ENDL;
    }
};

struct ScopedLoadFile : public ScopedFile {
    ScopedLoadFile (const std::string &filename)
      : ScopedFile(DGifOpenFileName(filename.c_str(), &openErr), filename)
    {
        if (file == nullptr) {
            EXCEPT << "Libgif error while opening file: "
                   << filename << " (" << GifErrorString(openErr) << ")" << ENDL;
        }
    }
    ~ScopedLoadFile (void)
    {
        if (file != nullptr) {
            if (DGifCloseFile(file, &openErr) == GIF_ERROR) {
                CERR << "Libgif error while closing file: "
                     << filename << " (" << GifErrorString(openErr) << ")" << std::endl;
            }
        }
    }
};

struct ScopedSaveFile : public ScopedFile {
    ScopedSaveFile (const std::string &filename)
      : ScopedFile(EGifOpenFileName(filename.c_str(), false, &openErr), filename)
    {
        if (file == nullptr) {
            EXCEPT << "Libgif error while opening file: "
                   << filename << " (" << GifErrorString(openErr) << ")" << ENDL;
        }
    }
    ~ScopedSaveFile (void)
    {
        if (file != nullptr) {
            if (EGifCloseFile(file, &openErr) == GIF_ERROR) {
                CERR << "Libgif error while closing file: "
                     << filename << " (" << GifErrorString(openErr) << ")" << std::endl;
            }
        }
    }
};

void gif_save (const std::string &filename, const GifFile &content)
{
    const std::string msg = "While writing \"" + filename + "\": ";
    const auto &frames = content.frames;

    // Sanity checks
    if (content.loops > 65536) 
        EXCEPT << msg << "Too many loops: " << content.loops << ENDL;
    if (frames.size() < 1)
        EXCEPT << msg << "Cannot write zero frames" << ENDL;
    uimglen_t w = frames[0].image->width;
    uimglen_t h = frames[0].image->height;
    for (unsigned i=0 ; i<frames.size() ; ++i) {
        if (frames[i].image->width != w || frames[i].image->height != h)
            EXCEPT << msg << "frame " << i << " has wrong dimensions" << ENDL;
        if (frames[i].image->colourChannels() != 3)
            EXCEPT << msg << "Frame " << i << " does not have 3 channels" << ENDL;
    }

    ScopedSaveFile f(filename);

    EGifSetGifVersion(f.file, true);

    if (EGifPutScreenDesc(f.file, w, h, 8, 0, nullptr) == GIF_ERROR)
        f.throwErr("writing screen desc");

    if (content.loops != 1) {
        unsigned short reps = content.loops;
        if (reps > 0) reps--;
        GifByteType nab[] = { 'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0' };
        if (EGifPutExtensionLeader(f.file, 255) == GIF_ERROR)
            f.throwErr("writing NAB leader");
        if (EGifPutExtensionBlock(f.file, sizeof nab, nab) == GIF_ERROR)
            f.throwErr("writing NAB block1");
        GifByteType loops[] = {
            1,
            GifByteType(reps & 0xff),
            GifByteType(reps >> 8)
        };
        if (EGifPutExtensionBlock(f.file, sizeof loops, loops) == GIF_ERROR)
            f.throwErr("writing NAB block2");
        if (EGifPutExtensionTrailer(f.file) == GIF_ERROR)
            f.throwErr("writing NAB trailer");
    }

    std::vector<GifByteType> rbuf(w * h);
    std::vector<GifByteType> gbuf(w * h);
    std::vector<GifByteType> bbuf(w * h);
    std::vector<bool> abuf(w * h);
    std::vector<GifByteType> obuf(w * h);

    for (const auto &frame : frames) {
        ColorMapPtr palette(256);
        GifColorType *pcols = palette.palette->Colors;
        int psize;
        if (frame.image->hasAlpha()) {
            const auto &img = *static_cast<const Image<3,1>*>(frame.image);
            psize = 255;
            for (uimglen_t y=0 ; y<h ; ++y) {
                for (uimglen_t x=0 ; x<w ; ++x) {
                    const auto &pixel = img.pixel(x, h-y-1);
                    if (pixel[3] < 0.5) {
                        rbuf[y * w + x] = 0;
                        gbuf[y * w + x] = 0;
                        bbuf[y * w + x] = 0;
                        abuf[y * w + x] = false;
                    } else {
                        rbuf[y * w + x] = clamp(pixel[0]) * 255 + 0.5;
                        gbuf[y * w + x] = clamp(pixel[1]) * 255 + 0.5;
                        bbuf[y * w + x] = clamp(pixel[2]) * 255 + 0.5;
                        abuf[y * w + x] = true;
                    }
                }
            }
        } else {
            psize = 256;
            const auto &img = *static_cast<const Image<3,0>*>(frame.image);
            for (uimglen_t y=0 ; y<h ; ++y) {
                for (uimglen_t x=0 ; x<w ; ++x) {
                    const auto &pixel = img.pixel(x, h-y-1);
                    rbuf[y * w + x] = clamp(pixel[0]) * 255 + 0.5;
                    gbuf[y * w + x] = clamp(pixel[1]) * 255 + 0.5;
                    bbuf[y * w + x] = clamp(pixel[2]) * 255 + 0.5;
                }
            }
        }
        
        if (GifQuantizeBuffer(w, h, &psize, &rbuf[0], &gbuf[0], &bbuf[0],
                              &obuf[0], pcols) == GIF_ERROR) {
            f.throwErr("quantising image");
        }

        // Use 255 for transparent colour.
        if (frame.image->hasAlpha()) {
            const auto &img = *static_cast<const Image<3,1>*>(frame.image);
            for (uimglen_t y=0 ; y<h ; ++y) {
                for (uimglen_t x=0 ; x<w ; ++x) {
                    const auto &pixel = img.pixel(x, h-y-1);
                    if (pixel[3] < 0.5)
                        obuf[y * w + x] = 255;
                }
            }
        }

        GraphicsControlBlock gdb {
            DISPOSAL_UNSPECIFIED,
            false,
            int(frame.delay / 0.01 + 0.5),  // Round to nearest multiple of 0.01
            frame.image->hasAlpha() ? 255 : NO_TRANSPARENT_COLOR
        };
        GifByteType ext[4];
        EGifGCBToExtension(&gdb, ext);
        if (EGifPutExtension(f.file, 0xf9, sizeof ext, ext) == GIF_ERROR)
            f.throwErr("writing GCE");

        if (EGifPutImageDesc(f.file, 0, 0, w, h, false, palette.palette) == GIF_ERROR)
            f.throwErr("writing image desc");

        for (uimglen_t y=0 ; y<h ; ++y) {
            if (EGifPutLine(f.file, &obuf[y * w], w) == GIF_ERROR)
                f.throwErr("writing pixel line");
        }
    }
}


GifFile gif_open (const std::string &filename)
{
    const std::string msg = "while reading \"" + filename + "\": ";
    ScopedLoadFile f(filename);

    uimglen_t sw = f.file->SWidth;
    uimglen_t sh = f.file->SHeight;

    GifRecordType record_type;

    std::vector<std::unique_ptr<Image<3, 1>>> imgs;
    std::vector<double> delays;

    int trans_colour = -1;
    int bg_colour = f.file->SBackGroundColor;

    unsigned loops = 1;
    double delay = 1;
    
    do {
        if (DGifGetRecordType(f.file, &record_type) == GIF_ERROR)
            f.throwErr("getting record type");

        switch (record_type) {
            case IMAGE_DESC_RECORD_TYPE: {
                if (DGifGetImageDesc(f.file) == GIF_ERROR)
                    f.throwErr("getting image desc");

                ColorMapObject *palette =
                    f.file->Image.ColorMap ? f.file->Image.ColorMap : f.file->SColorMap;

                Colour<3, true> bg(0);
                if (bg_colour == trans_colour) {
                    bg[0] = 0;
                    bg[1] = 0;
                    bg[2] = 0;
                    bg[3] = 0;
                } else {
                    bg[0] = palette->Colors[bg_colour].Red / 255.0;
                    bg[1] = palette->Colors[bg_colour].Green / 255.0;
                    bg[2] = palette->Colors[bg_colour].Blue / 255.0;
                    bg[3] = 1;
                }
                imgs.emplace_back(new Image<3, 1>(sw, sh));
                delays.emplace_back(delay);
                auto &img = *imgs[imgs.size() - 1];
                for (uimglen_t y=0 ; y<sh ; ++y) {
                    for (uimglen_t x=0 ; x<sw ; ++x) {
                        img.pixel(x, sh-y-1) = bg;
                    }
                }
                // Dimensions of frame
                uimglen_t top = f.file->Image.Top;
                uimglen_t fh = f.file->Image.Height;
                uimglen_t bottom = top + fh;
                uimglen_t left = f.file->Image.Left;
                uimglen_t fw = f.file->Image.Width;
                uimglen_t right = left + fw;
                std::vector<GifByteType> line(fw);
                if (f.file->Image.Interlace) {
                    // This is a mechanism for progressive rendering.
                    const uimglen_t offsets[] = { 0, 4, 2, 1 };
                    const uimglen_t jumps[] = { 8, 8, 4, 2 };
                    // Need to perform 4 passes on the images:
                    for (uimglen_t i = 0; i < 4; i++) {
                        for (uimglen_t y = top + offsets[i] ; y < top+fh ; y += jumps[i]) {
                            if (DGifGetLine(f.file, &line[0], fw) == GIF_ERROR)
                                f.throwErr("getting a line of pixels");
                            for (uimglen_t x=left ; x<right && x<sw ; ++x) {
                                auto &pixel = img.pixel(x, sh-y-1);
                                int colour = line[x - left];
                                if (colour == trans_colour) {
                                    pixel[0] = 0;
                                    pixel[1] = 0;
                                    pixel[2] = 0;
                                    pixel[3] = 0;
                                } else {
                                    pixel[0] = palette->Colors[colour].Red / 255.0;
                                    pixel[1] = palette->Colors[colour].Green / 255.0;
                                    pixel[2] = palette->Colors[colour].Blue / 255.0;
                                    pixel[3] = 1;
                                }
                            }
                        }
                    }
                } else {
                    for (uimglen_t y=top ; y<bottom && y<sh ; ++y) {
                        if (DGifGetLine(f.file, &line[0], fw) == GIF_ERROR)
                            f.throwErr("getting a line of pixels");
                        for (uimglen_t x=left ; x<right && x<sw ; ++x) {
                            auto &pixel = img.pixel(x, sh-y-1);
                            int colour = line[x - left];
                            if (colour == trans_colour) {
                                pixel[0] = 0;
                                pixel[1] = 0;
                                pixel[2] = 0;
                                pixel[3] = 0;
                            } else {
                                pixel[0] = palette->Colors[colour].Red / 255.0;
                                pixel[1] = palette->Colors[colour].Green / 255.0;
                                pixel[2] = palette->Colors[colour].Blue / 255.0;
                                pixel[3] = 1;
                            }
                        }
                    }
                }
            }
            break;

            case EXTENSION_RECORD_TYPE: {
                int ext_code;
                GifByteType *ext;
                std::vector<std::vector<GifByteType>> blocks;
                if (DGifGetExtension(f.file, &ext_code, &ext) == GIF_ERROR)
                    f.throwErr("getting extension");
                while (ext != NULL) {
                    unsigned block_sz = ext[0];
                    blocks.emplace_back(block_sz);
                    for (unsigned i=0 ; i<block_sz ; ++i)
                        blocks[blocks.size() - 1][i] = ext[1 + i];
                    if (DGifGetExtensionNext(f.file, &ext) == GIF_ERROR)
                        f.throwErr("getting next extension");
                }

                switch (ext_code) {
                    case 249: {  // Graphics control extension
                        if (blocks.size() < 1) goto ignore;
                        if (blocks[0].size() < 4) goto ignore;
                        int flags = GifByteType(blocks[0][0]);
                        bool trans = (flags >> 0) & 0x1;
                        bool ui = (flags >> 1) & 0x2;
                        unsigned disposal = (flags >> 2) & 0x7;
                        int block_delay = GifByteType(blocks[0][1]) | GifByteType(blocks[0][2] << 8);
                        int block_trans_colour = GifByteType(blocks[0][3]);
                        trans_colour = trans ? block_trans_colour : -1;
                        delay = block_delay * 0.01;
                        (void) ui;
                        (void) disposal;
                    } break;
                    case 254: {  // Comment extension
                        for (unsigned i=0 ; i<blocks.size() ; ++i) {
                            // std::cout << blocks[i] << std::endl;
                        }
                    } break;
                    case 255: {  // Application extension
                        std::vector<GifByteType> netscape2_0 =
                            { 'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0' };
                        if (netscape2_0 == blocks[0]) {
                            if (blocks.size() < 2) goto ignore;
                            if (blocks[1].size() < 3) goto ignore;
                            unsigned repeats = blocks[1][1] | (blocks[1][2] << 8);
                            loops = repeats == 0 ? 0 : repeats + 1ul;
                        }
                    } break;
                    default: {
                        std::cout << "Unrecognised extension code: " << ext_code << std::endl;
                    }
                }
                ignore:;
            }
            break;

            case TERMINATE_RECORD_TYPE:
            goto end;

            default:
            EXCEPTEX << "Internal error." << ENDL;
        }
    } while (true);
    end:;

    GifFile r;
    r.loops = loops;
    r.frames.resize(imgs.size());
    for (unsigned i=0 ; i<imgs.size() ; ++i) {
        r.frames[i].image = imgs[i].release();
        r.frames[i].delay = delays[i];
    }

    return r;
}

