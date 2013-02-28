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

struct RGB;
class Image;

#ifndef IMAGE_H
#define IMAGE_H

struct RGB {
    float r, g, b;
};


class Image {

    RGB * data;

    public:

    const unsigned height, width;
    Image (unsigned height, unsigned width)
      : height(height), width(width)
    {
        data = new RGB[size()];
    }

    ~Image (void)
    {
        delete data;
    }

    unsigned long size() { return (unsigned long)(height) * width; };

    RGB &pixel (unsigned x, unsigned y) { return data[y*width+x]; }
};

Image *image_load (const std::string &filename);

static inline std::ostream &operator<<(std::ostream &o, const RGB &p)
{
    o << "(" << p.r << ", " << p.g << ", " << p.b << ")";
    return o;
}

#endif
