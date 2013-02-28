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

#include <ostream>
#include <string>

template<int ch> struct Pixel;
template<int ch> class Image;
class ImageBase;

#ifndef IMAGE_H
#define IMAGE_H

/* Main purpose of this is to avoid C/C++ stupid array type syntax. */
template<int ch> struct Pixel {
    float v[ch];
    const float &operator[] (int i) const { return v[i]; }
    float &operator[] (int i) { return v[i]; }
};

static inline std::ostream &operator<<(std::ostream &o, const Pixel<3> &p)
{
    o << "(" << p[0] << ", " << p[1] << ", " << p[2] << ")";
    return o;
}


class ImageBase {

    public:

    const unsigned width, height;
    ImageBase (unsigned width, unsigned height)
      : width(width), height(height)
    {
    }

    unsigned long numPixels() { return (unsigned long)(height) * width; };

    virtual ~ImageBase (void) { }

    virtual int channels() = 0;
};

template<int ch> class Image : public ImageBase {

    Pixel<ch> * data;

    public:

    Image (unsigned width, unsigned height)
      : ImageBase(width, height)
    {
        data = new Pixel<ch>[numPixels()];
    }

    ~Image (void)
    {
        delete data;
    }

    Pixel<ch> &pixel (unsigned x, unsigned y) { return data[y*width+x]; }

    int channels (void) { return ch; }
};

ImageBase *image_load (const std::string &filename);

template<int ch> Image<ch> *image_make (unsigned width, unsigned height, float (&init)[ch])
{
    Image<ch> *my_image = new Image<ch>(width, height);

    for (unsigned y=0 ; y<my_image->height ; ++y) {
        for (unsigned x=0 ; x<my_image->width ; ++x) {
            for (unsigned c=0 ; c<ch ; ++c) {
                my_image->pixel(x, y)[c] = init[c];
            }
        }
    }

    return my_image;
    
}

#endif
