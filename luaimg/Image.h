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


class PixelBase;
template<unsigned ch> struct Pixel;
class ImageBase;
template<unsigned ch> class Image;

#ifndef IMAGE_H
#define IMAGE_H

struct PixelBase {
    virtual unsigned channels() = 0;
    virtual ~PixelBase (void) { }
};

/* Main purpose of this is to avoid C/C++ stupid array type syntax. */
template<unsigned ch> struct Pixel : PixelBase {
    Pixel () { }
    Pixel (float d) { for (unsigned c=0 ; c<ch ; ++c) v[c] = d; }
    float v[ch];
    unsigned channels() { return ch; }
    const float &operator[] (unsigned i) const { return v[i]; }
    float &operator[] (unsigned i) { return v[i]; }

    Pixel<ch> pow (float other) const { Pixel<ch> r; for (unsigned c=0 ; c<ch ; ++c) r[c] = powf((*this)[c],other); return r; }

    Pixel<ch> add (const Pixel<ch> &other) const { Pixel<ch> r; for (unsigned c=0 ; c<ch ; ++c) r[c] = (*this)[c] + other[c]; return r; }
    Pixel<ch> sub (const Pixel<ch> &other) const { Pixel<ch> r; for (unsigned c=0 ; c<ch ; ++c) r[c] = (*this)[c] - other[c]; return r; }
    Pixel<ch> mul (const Pixel<ch> &other) const { Pixel<ch> r; for (unsigned c=0 ; c<ch ; ++c) r[c] = (*this)[c] * other[c]; return r; }
    Pixel<ch> div (const Pixel<ch> &other) const { Pixel<ch> r; for (unsigned c=0 ; c<ch ; ++c) r[c] = (*this)[c] / other[c]; return r; }

    Pixel<ch> unm (void) { Pixel<ch> r; for (unsigned c=0 ; c<ch ; ++c) r[c] = -(*this)[c]; return r; }

    Pixel<ch> add (float other) const { Pixel<ch> r; for (unsigned c=0 ; c<ch ; ++c) r[c] = (*this)[c] + other; return r; }
    Pixel<ch> sub (float other) const { Pixel<ch> r; for (unsigned c=0 ; c<ch ; ++c) r[c] = (*this)[c] - other; return r; }
    Pixel<ch> mul (float other) const { Pixel<ch> r; for (unsigned c=0 ; c<ch ; ++c) r[c] = (*this)[c] * other; return r; }
    Pixel<ch> div (float other) const { Pixel<ch> r; for (unsigned c=0 ; c<ch ; ++c) r[c] = (*this)[c] / other; return r; }
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

    bool compatibleWith (ImageBase *other) {
        if (other->width != width) return false;
        if (other->height != height) return false;
        if (other->channels() != channels()) return false;
        return true;
    }

    virtual PixelBase &pixel (unsigned x, unsigned y) = 0;

    virtual unsigned channels() = 0;

    virtual float rms (ImageBase *other) = 0;

    virtual ImageBase *pow (float other) = 0;

    virtual ImageBase *add (ImageBase *other) = 0;
    virtual ImageBase *sub (ImageBase *other) = 0;
    virtual ImageBase *mul (ImageBase *other) = 0;
    virtual ImageBase *div (ImageBase *other) = 0;

    virtual ImageBase *unm (void) = 0;

    virtual ImageBase *add (const PixelBase &other) = 0;
    virtual ImageBase *sub (const PixelBase &other, bool swapped) = 0;
    virtual ImageBase *mul (const PixelBase &other) = 0;
    virtual ImageBase *div (const PixelBase &other, bool swapped) = 0;


    virtual ImageBase *crop (unsigned left, unsigned bottom, unsigned width, unsigned height) = 0;

};

static inline std::ostream &operator<<(std::ostream &o, ImageBase &img)
{
    o << "Image ("<<img.width<<","<<img.height<<")@"<<img.channels()<<" [0x"<<&img<<"]";
    return o;
}

template<unsigned ch> class Image : public ImageBase {

    Pixel<ch> * data;

    public:

    Image (unsigned width, unsigned height)
      : ImageBase(width, height)
    {
        data = new Pixel<ch>[numPixels()];
    }

    ~Image (void)
    {
        delete [] data;
    }

    virtual Pixel<ch> &pixel (unsigned x, unsigned y) { return data[y*width+x]; }

    virtual unsigned channels (void) { return ch; }

    virtual float rms (ImageBase *other_)
    {
        Image<ch> *other = static_cast<Image<ch>*>(other_);
        float ret = 0;
        for (unsigned y=0 ; y<height ; ++y) {
            for (unsigned x=0 ; x<width ; ++x) {
                for (unsigned c=0 ; c<ch ; ++c) {
                    float diff = this->pixel(x,y)[c] - other->pixel(x,y)[c];
                    ret += diff * diff;
                }
            }
        }
        return ret;
    }

    virtual Image<ch> *pow (float other)
    {
        Image<ch> *ret = new Image<ch>(width, height);
        for (unsigned y=0 ; y<height ; ++y) {
            for (unsigned x=0 ; x<width ; ++x) {
                ret->pixel(x,y) = this->pixel(x,y).pow(other);
            }
        }
        return ret;
    }

    virtual Image<ch> *add (ImageBase *other_)
    {
        Image<ch> *other = static_cast<Image<ch>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (unsigned y=0 ; y<height ; ++y) {
            for (unsigned x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).add(other->pixel(x, y));
            }
        }
        return ret;
    }
    virtual Image<ch> *sub (ImageBase *other_)
    {
        Image<ch> *other = static_cast<Image<ch>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (unsigned y=0 ; y<height ; ++y) {
            for (unsigned x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).sub(other->pixel(x, y));
            }
        }
        return ret;
    }
    virtual Image<ch> *mul (ImageBase *other_)
    {
        Image<ch> *other = static_cast<Image<ch>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (unsigned y=0 ; y<height ; ++y) {
            for (unsigned x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).mul(other->pixel(x, y));
            }
        }
        return ret;
    }
    virtual Image<ch> *div (ImageBase *other_)
    {
        Image<ch> *other = static_cast<Image<ch>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (unsigned y=0 ; y<height ; ++y) {
            for (unsigned x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).div(other->pixel(x, y));
            }
        }
        return ret;
    }

    virtual Image<ch> *unm (void)
    {
        Image<ch> *ret = new Image<ch>(width, height);
        for (unsigned y=0 ; y<height ; ++y) {
            for (unsigned x=0 ; x<width ; ++x) {
                ret->pixel(x,y) = this->pixel(x,y).unm();
            }
        }
        return ret;
    }

    virtual Image<ch> *add (const PixelBase &p_)
    {
        const Pixel<ch> &p = *static_cast<Pixel<ch> const *>(&p_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (unsigned y=0 ; y<height ; ++y) {
            for (unsigned x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).add(p);
            }
        }
        return ret;
    }
    virtual Image<ch> *sub (const PixelBase &p_, bool swapped)
    {
        const Pixel<ch> &p = *static_cast<Pixel<ch> const *>(&p_);
        Image<ch> *ret = new Image<ch>(width, height);
        if (swapped) {
            for (unsigned y=0 ; y<height ; ++y) {
                for (unsigned x=0 ; x<width ; ++x) {
                    ret->pixel(x, y) = p.sub(this->pixel(x, y));
                }
            }
        } else {
            for (unsigned y=0 ; y<height ; ++y) {
                for (unsigned x=0 ; x<width ; ++x) {
                    ret->pixel(x, y) = this->pixel(x, y).sub(p);
                }
            }
        }
        return ret;
    }
    virtual Image<ch> *mul (const PixelBase &p_)
    {
        const Pixel<ch> &p = *static_cast<Pixel<ch> const *>(&p_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (unsigned y=0 ; y<height ; ++y) {
            for (unsigned x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).mul(p);
            }
        }
        return ret;
    }
    virtual Image<ch> *div (const PixelBase &p_, bool swapped)
    {
        const Pixel<ch> &p = *static_cast<Pixel<ch> const *>(&p_);
        Image<ch> *ret = new Image<ch>(width, height);
        if (swapped) {
            for (unsigned y=0 ; y<height ; ++y) {
                for (unsigned x=0 ; x<width ; ++x) {
                    ret->pixel(x, y) = p.div(this->pixel(x, y));
                }
            }
        } else {
            for (unsigned y=0 ; y<height ; ++y) {
                for (unsigned x=0 ; x<width ; ++x) {
                    ret->pixel(x, y) = this->pixel(x, y).div(p);
                }
            }
        }
        return ret;
    }


    virtual ImageBase *crop (unsigned left, unsigned bottom, unsigned w, unsigned h)
    {
        Image<ch> *ret = new Image<ch>(w, h);
        for (unsigned y=0 ; y<height ; ++y) {
            for (unsigned x=0 ; x<width ; ++x) {
                ret->pixel(x,y) = this->pixel(x+left,y+bottom);
            }
        }
        return ret;
    }

};

ImageBase *image_load (const std::string &filename);

bool image_save (ImageBase *image, const std::string &filename);

template<unsigned ch> Image<ch> *image_make (unsigned width, unsigned height, float (&init)[ch])
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
