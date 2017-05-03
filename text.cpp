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
#include <cstdio>
#include <cmath>
#include <cstring>

#include <string>
#include <iostream>
#include <fstream>

#include <ft2build.h>
// wrap in #ifdef to keep makedepend happy
#ifdef FT_FREETYPE_H
#include FT_FREETYPE_H
#endif

FT_Library ft2;

#include <lua_util.h>
#include <unicode_util.h>
#include <exception.h>

#include "text.h"
#include "image.h"

void text_init (void)
{
    if (0 != FT_Init_FreeType(&ft2)) {
        EXCEPT << "Couldn't initialise the FreeType2 font library." << std::endl;
    }
}

Image<1,0> *make_text_codepoint (const std::string &font, uimglen_t font_w, uimglen_t font_h, unsigned long cp)
{
    FT_Face face;
    int error = FT_New_Face(ft2, font.c_str(), 0, &face);
    if (error == FT_Err_Unknown_File_Format) {
        EXCEPT<<"Unknown file format reading: "<<font<<std::endl;
    } else if (error != 0) {
        EXCEPT<<"Could not read "<<font<<": "<<error<<std::endl;
    }

    error = FT_Set_Pixel_Sizes(face, font_w, font_h);
    if (0 != error) {
        FT_Done_Face(face);
        EXCEPT<<"Could not set font size ("<<font_w<<","<<font_h<<") for font "<<font<<": "<<error<<std::endl;
    }
    
    error = FT_Load_Char(face, cp, FT_LOAD_RENDER);
    if (0 != error) {
        FT_Done_Face(face);
        EXCEPT<<"Could not load glyph "<<cp<<" for font "<<font<<": "<<error<<std::endl;
    }

    // record max in here
    simglen_t max_x = (face->glyph->advance.x-1) / 64;
    simglen_t max_y = face->size->metrics.ascender/64;
    simglen_t min_x = 0; // always zero
    simglen_t min_y = (face->size->metrics.descender+1) / 64;

    //std::cout << "(" << min_x << "," << min_y << ") (" << max_x << "," << max_y << ")" << std::endl;
    //std::cout << bool(FT_IS_SCALABLE(face)) << std::endl;

    Colour<1,0> bg(0);
    Colour<1,1> fg(1);
    Image<1,0> *img = image_make<1,0>(max_x-min_x+1, max_y-min_y+1, bg);

    if (face->glyph->bitmap.num_grays == 1) {
        for (uimglen_t p=0 ; p<face->glyph->bitmap.width ; p++) {
            for (uimglen_t q=0 ; q<face->glyph->bitmap.rows ; q++) {
                unsigned char byte = (unsigned char)(face->glyph->bitmap.buffer[q * face->glyph->bitmap.pitch + p/8]);
                unsigned char v = (byte >> (7-(p % 8))) & 1;
                img->drawPixelSafe(simglen_t(face->glyph->bitmap_left) + p - min_x,
                                   simglen_t(face->glyph->bitmap_top) - q - min_y,
                                   &fg,
                                   float(v));
            }
        }
    } else if (face->glyph->bitmap.num_grays == 256) {
        for (uimglen_t p=0 ; p<face->glyph->bitmap.width ; p++) {
            for (uimglen_t q=0 ; q<face->glyph->bitmap.rows ; q++) {
                img->drawPixelSafe(simglen_t(face->glyph->bitmap_left) + p - min_x,
                                   simglen_t(face->glyph->bitmap_top) - q - min_y,
                                   &fg,
                                   (unsigned char)(face->glyph->bitmap.buffer[q * face->glyph->bitmap.pitch + p]) / 255.0f);
            }
        }
    } else {
        FT_Done_Face(face);
        EXCEPTEX<<font<<": "<<face->glyph->bitmap.num_grays<<std::endl;
    }

    FT_Done_Face(face);

    return img;
}

Image<1,0> *make_text (const std::string &font, uimglen_t font_w, uimglen_t font_h, const std::string &text,
                       float xx, float xy, float yx, float yy)
{
    FT_Face face;
    int error = FT_New_Face(ft2, font.c_str(), 0, &face);
    if (error == FT_Err_Unknown_File_Format) {
        EXCEPT<<"Unknown file format reading: "<<font<<std::endl;
    } else if (error != 0) {
        EXCEPT<<"Could not read "<<font<<": "<<error<<std::endl;
    }

    error = FT_Set_Pixel_Sizes(face, font_w, font_h);
    if (0 != error) {
        FT_Done_Face(face);
        EXCEPT<<"Could not set font size ("<<font_w<<","<<font_h<<") for font "<<font<<": "<<error<<std::endl;
    }
    
    FT_Matrix     matrix;
    matrix.xx = 0x10000L * xx;
    matrix.xy = 0x10000L * xy;
    matrix.yx = 0x10000L * yx;
    matrix.yy = 0x10000L * yy;

    FT_Vector pen;
    pen.x = 0;
    pen.y = 0;

    // record max in here
    simglen_t max_x = 0;
    simglen_t max_y = 0;
    simglen_t min_x = 0;
    simglen_t min_y = 0;

    // calculate size
    for (size_t i=0 ; i<text.length() ; ++i) {
        unsigned long cp = decode_utf8(text, i);

        FT_Set_Transform(face, &matrix, &pen);

        error = FT_Load_Char(face, cp, FT_LOAD_RENDER);
        if (0 != error) {
            FT_Done_Face(face);
            EXCEPT<<"Could not load glyph "<<cp<<" for font "<<font<<": "<<error<<std::endl;
        }

        for (uimglen_t p=0 ; p<face->glyph->bitmap.width ; p++) {
            for (uimglen_t q=0 ; q<face->glyph->bitmap.rows ; q++) {
                simglen_t x = simglen_t(face->glyph->bitmap_left) + p;
                simglen_t y = simglen_t(face->glyph->bitmap_top) - q;
                if (!FT_IS_SCALABLE(face)) {
                    x += pen.x / 64;
                    y += pen.y / 64;
                }
                if (x > max_x) max_x = x;
                if (y > max_y) max_y = y;
                if (x < min_x) min_x = x;
                if (y < min_y) min_y = y;
            }
        }

        pen.x += face->glyph->advance.x;
        pen.y += face->glyph->advance.y;
    }

    Colour<1,0> bg(0);
    Colour<1,1> fg(1);
    Image<1,0> *img = image_make<1,0>(max_x-min_x+1, max_y-min_y+1, bg);

    pen.x = 0;
    pen.y = 0;

    // draw
    for (size_t i=0 ; i<text.length() ; ++i) {

        FT_Set_Transform(face, &matrix, &pen);

        unsigned long cp = decode_utf8(text, i);

        error = FT_Load_Char(face, cp, FT_LOAD_RENDER);
        if (0 != error) {
            FT_Done_Face(face);
            EXCEPT<<"Could not load glyph "<<cp<<" for font "<<font<<": "<<error<<std::endl;
        }

        if (face->glyph->bitmap.num_grays == 1) {
            for (uimglen_t p=0 ; p<face->glyph->bitmap.width ; p++) {
                for (uimglen_t q=0 ; q<face->glyph->bitmap.rows ; q++) {
                    unsigned char byte = (unsigned char)(face->glyph->bitmap.buffer[q * face->glyph->bitmap.pitch + p/8]);
                    unsigned char v = (byte >> (7-(p % 8))) & 1;
                    img->drawPixelSafe(simglen_t(face->glyph->bitmap_left) + p - min_x + simglen_t(pen.x/64),
                                       simglen_t(face->glyph->bitmap_top) - q - min_y + simglen_t(pen.y/64),
                                       &fg,
                                       float(v));
                }
            }
        } else if (face->glyph->bitmap.num_grays == 256) {
            for (uimglen_t p=0 ; p<face->glyph->bitmap.width ; p++) {
                for (uimglen_t q=0 ; q<face->glyph->bitmap.rows ; q++) {
                    img->drawPixelSafe(simglen_t(face->glyph->bitmap_left) + p - min_x,
                                       simglen_t(face->glyph->bitmap_top) - q - min_y,
                                       &fg,
                                       (unsigned char)(face->glyph->bitmap.buffer[q * face->glyph->bitmap.pitch + p]) / 255.0f);
                }
            }
        } else {
            FT_Done_Face(face);
            EXCEPTEX<<font<<": "<<face->glyph->bitmap.num_grays<<std::endl;
        }

        pen.x += face->glyph->advance.x;
        pen.y += face->glyph->advance.y;

    }

    FT_Done_Face(face);

    return img;
}
