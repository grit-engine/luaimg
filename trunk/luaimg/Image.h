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

#ifdef WIN32
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef signed int int32_t;
typedef signed char int8_t;
#else
#include <cinttypes>
#endif

typedef uint32_t uimglen_t;
typedef int32_t simglen_t;
typedef uint8_t chan_t; 
struct ColourBase;
template<chan_t ch, chan_t ach> struct Colour;
class ImageBase;
template<chan_t ch, chan_t ach> class Image;

void RGBtoHSL (float R, float G, float B, float &H, float &S, float &L);
void HSLtoRGB (float H, float S, float L, float &R, float &G, float &B);
void HSLtoHSV (float HH, float SS, float LL, float &H, float &S, float &L);
void HSVtoHSL (float H, float S, float L, float &HH, float &SS, float &LL);
void RGBtoHSV (float R, float G, float B, float &H, float &S, float &L);
void HSVtoRGB (float H, float S, float L, float &R, float &G, float &B);

#ifndef IMAGE_H
#define IMAGE_H

#define PI 3.1415926535897932385

#include <cmath>

#include <ostream>
#include <string>

static inline simglen_t mymod (simglen_t a, simglen_t b)
{
    simglen_t c = a % b;
    // maybe -b < c <= 0
    c = (c + b) % b;
    return c;
}

enum ScaleFilter {
    SF_BOX,
    SF_BILINEAR,
    SF_BSPLINE,
    SF_BICUBIC,
    SF_CATMULLROM,
    SF_LANCZOS3
};

struct ColourBase {
/*
    virtual chan_t channels() const = 0;
    virtual bool hasAlpha() const = 0;
    virtual chan_t channelsNonAlpha() const = 0;
    virtual float *raw (void) = 0;
    virtual const float *raw (void) const = 0;
    virtual ~ColourBase (void) { }
*/
};

// ach can be 0 or 1
template<chan_t ch, chan_t ach> struct Colour : ColourBase {

    chan_t channels() const { return ch+ach; }
    bool hasAlpha() const { return ach==1; }
    chan_t channelsNonAlpha() const { return ch; }

    Colour () { }

    Colour (float d)
    {
        for (chan_t c=0 ; c<ch+ach ; ++c)
            v[c] = d;
    }

    Colour (float d, float a)
    {
        for (chan_t c=0 ; c<ch ; ++c)
            v[c] = d;
    }

    Colour (const Colour<1,ach> &d)
    {
        for (chan_t c=0 ; c<ch ; ++c)
            v[c] = d[0];
        if (hasAlpha())
            v[ch] = d[1];
    }

    float v[ch+ach];

    float *raw (void) { return &v[0]; }
    const float *raw (void) const { return &v[0]; }

    Colour<ch,ach> &pixel (uimglen_t, uimglen_t) { return *this; }
    const Colour<ch,ach> &pixel (uimglen_t, uimglen_t) const { return *this; }

    const float &operator[] (chan_t i) const { return v[i]; }
    float &operator[] (chan_t i) { return v[i]; }

    Colour<ch,ach> unm (void) const
    {
        Colour<ch,ach> r;
        for (chan_t c=0 ; c<ch ; ++c)
            r[c] = -(*this)[c];
        if (ach == 1) r[ch] = (*this)[ch];
        return r;
    }
    Colour<ch,ach> abs (void) const
    {
        Colour<ch,ach> r;
        for (chan_t c=0 ; c<ch ; ++c)
            r[c] = fabsf((*this)[c]);
        if (ach == 1) r[ch] = (*this)[ch];
        return r;
    }
};

// TODO: need to find a way to 

// a + b: a's alpha channel moderates a's effect on b.  b's alpha channel is used in the result.
// lack of alpha channel implies an alpha value of 1
template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float op(float,float)>
Colour<ch2,ach2> colour_zip (const Colour<ch1,ach1> &a, const Colour<ch2, ach2> &b)
{
    if (ch2 != ch1) abort();
    Colour<ch2, ach2> r;
    if (ach1 == 1) {
        float alpha = a[ch2];
        for (chan_t c=0 ; c<ch2 ; ++c) {
            r[c] = (1-alpha)*b[c] + alpha*op(a[c], b[c]);
        }
    } else {
        for (chan_t c=0 ; c<ch2 ; ++c) {
            r[c] = op(a[c], b[c]);
        }
    }
    if (ach2 == 1) r[ch2] = b[ch2];
    return r;
}

// regular compose op -- simple alpha blend
template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
Colour<ch2,ach2> colour_blend (const Colour<ch1,ach1> &a, const Colour<ch2,ach2> &b)
{
    if (ch2 != ch1) abort();
    Colour<ch2, ach2> r;
    if (ach1 == 1) {
        if (ach2 == 1) {
            float alpha = std::max(0.0f, std::min(1.0f, a[ch2]));
            float old_alpha = std::max(0.0f, std::min(1.0f, b[ch2]));
            float new_alpha = 1 - (1-alpha)*(1-old_alpha);
            if (alpha == 0) {
                for (chan_t c=0 ; c<ch2 ; ++c) {
                    r[c] = b[c];
                }
            } else {
                for (chan_t c=0 ; c<ch2 ; ++c) {
                    r[c] = alpha/new_alpha*a[c] + (1-alpha/new_alpha)*b[c];
                }
            }
            r[ch2] = new_alpha;
        } else {
            float alpha = std::max(0.0f, std::min(1.0f, a[ch2]));
            for (chan_t c=0 ; c<ch2 ; ++c) {
                r[c] = alpha*a[c] + (1-alpha)*b[c];
            }
        }
    } else {
        for (chan_t c=0 ; c<ch2 ; ++c) {
            r[c] = a[c];
        }
        if (ach2==1) {
            r[ch2] = b[ch2];
        }
    }
    return r;
}


// regular compose op -- simple alpha blend
template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
Colour<ch2,ach2> colour_lerp (const Colour<ch1,ach1> &a, const Colour<ch2,ach2> &b, float param)
{
    if (ch2 != ch1) abort();
    if (ach2 != ach1) abort();
    Colour<ch2,ach2> r;
    for (chan_t c=0 ; c<ch2+ach2 ; ++c) {
        r[c] = (1-param)*a[c] + param*b[c];
    }
    return r;
}


class ImageBase {

    public:

    virtual chan_t channels (void) const = 0;
    virtual bool hasAlpha (void) const = 0;
    virtual chan_t channelsNonAlpha() const = 0;

    const uimglen_t width, height;

    ImageBase (uimglen_t width, uimglen_t height)
      : width(width), height(height)
    {
    }

    unsigned long numPixels() const { return (unsigned long)(height) * width; };
    unsigned long numBytes() const { return numPixels()*4; }

    virtual float *raw (void) = 0;
    virtual const float *raw (void) const = 0;

    virtual ~ImageBase (void) { }

    bool sizeCompatibleWith (const ImageBase *other) const {
        if (other->width != width) return false;
        if (other->height != height) return false;
        return true;
    }

    virtual ColourBase &pixelSlow (uimglen_t x, uimglen_t y) = 0;
    virtual const ColourBase &pixelSlow (uimglen_t x, uimglen_t y) const = 0;

    virtual ImageBase *unm (void) const = 0;
    virtual ImageBase *abs (void) const = 0;

    virtual ImageBase *clone (bool flip_x, bool flip_y) const = 0;
    virtual ImageBase *normalise (void) const = 0;

    virtual ImageBase *scale (uimglen_t width, uimglen_t height, ScaleFilter filter) const;
    virtual ImageBase *rotate (float angle, const ColourBase *bg_) const = 0;
    virtual ImageBase *crop (simglen_t left, simglen_t bottom, uimglen_t w, uimglen_t h, const ColourBase *bg) const = 0;

    virtual void drawImage (const ImageBase *src_, simglen_t left, simglen_t bottom) = 0;
    virtual ImageBase *convolve (const Image<1,0> *kernel, bool wrap_x, bool wrap_y) const = 0;

};

static inline std::ostream &operator<<(std::ostream &o, ImageBase &img)
{
    o << "Image ("<<img.width<<","<<img.height<<")x"<<int(img.channels())<<(img.hasAlpha()?"A":"")<<" [0x"<<&img<<"]";
    return o;
}

template<> class Image<0,0> : public ImageBase { };

template<chan_t ch, chan_t ach> class Image : public ImageBase {

    Colour<ch, ach> *data;

    public:

    chan_t channels() const { return ch+ach; }
    bool hasAlpha() const { return ach==1; }
    chan_t channelsNonAlpha() const { return ch; }

    Image (uimglen_t width, uimglen_t height)
      : ImageBase(width, height)
    {
        data = new Colour<ch, ach>[numPixels()];
    }

    ~Image (void)
    {
        delete [] data;
    }

    float *raw (void) { return data[0].raw(); }
    const float *raw (void) const { return data[0].raw(); }

    Colour<ch,ach> &pixel (uimglen_t x, uimglen_t y) { return data[y*width+x]; }
    const Colour<ch,ach> &pixel (uimglen_t x, uimglen_t y) const { return data[y*width+x]; }

    Colour<ch,ach> &pixelSlow (uimglen_t x, uimglen_t y) { return pixel(x,y); }
    const Colour<ch,ach> &pixelSlow (uimglen_t x, uimglen_t y) const { return pixel(x,y); }

    Colour<ch,ach> pixelSafe (float x, float y, const Colour<ch,ach> &bg) const
    {
        if (x < 0 || y < 0) return bg;
        if (x >= width || y >= height) return bg;
        return pixel(x, y);
    }

    Image<ch,ach> *unm (void) const
    {
        Image<ch,ach> *ret = new Image<ch,ach>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x,y) = this->pixel(x,y).unm();
            }
        }
        return ret;
    }

    Image<ch,ach> *abs (void) const
    {
        Image<ch,ach> *ret = new Image<ch,ach>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x,y) = this->pixel(x,y).abs();
            }
        }
        return ret;
    }



    Image<ch,ach> *crop (simglen_t left, simglen_t bottom, uimglen_t w, uimglen_t h, const ColourBase *bg_) const
    {
        Image<ch, ach> *ret = new Image<ch, ach>(w, h);
        if (bg_ == NULL) {
            for (uimglen_t y=0 ; y<h ; ++y) {
                for (uimglen_t x=0 ; x<w ; ++x) {
                    uimglen_t old_x = mymod(x+left, width);
                    uimglen_t old_y = mymod(y+bottom, height);
                    ret->pixel(x,y) = this->pixel(old_x, old_y);
                }
            }
        } else {
            const Colour<ch, ach> &bg = *static_cast<const Colour<ch,ach>*>(bg_);
            for (uimglen_t y=0 ; y<h ; ++y) {
                for (uimglen_t x=0 ; x<w ; ++x) {
                    uimglen_t old_x = x+left;
                    uimglen_t old_y = y+bottom;
                    ret->pixel(x,y) = (old_x<width && old_y<height) ? this->pixel(old_x, old_y) : bg;
                }
            }
        }
        return ret;
    }

/*
    -------------+-------------
    |            |            |
    |            |            |
  --+------------|------------+--
    |            |            |
    |            |            |
    -------------+-------------
*/
    Image<ch,ach> *rotate (float angle, const ColourBase *bg_) const
    {
        // calculate these later
        Colour<ch,ach> bg(0);
        if (bg_ != NULL) bg = *static_cast<const Colour<ch,ach>*>(bg_);
        float s = ::sin(angle*PI/180);
        float c = ::cos(angle*PI/180);
        uimglen_t w = (fabs(c)*width + fabs(s)*height + 0.5);
        uimglen_t h = (fabs(s)*width + fabs(c)*height + 0.5);
        Image<ch, ach> *ret = new Image<ch, ach>(w, h);
        for (uimglen_t y=0 ; y<h; ++y) {
            for (uimglen_t x=0 ; x<w; ++x) {
                float rel_x = float(x) - w/2.0f + 0.5f; // 0.5
                float rel_y = float(y) - h/2.0f + 0.5f; // -2 to 2
                float src_x = c*rel_x - s*rel_y + width/2.0f; // 0.5
                float src_y = s*rel_x + c*rel_y + height/2.0f; // 0.5 to 4.5
                if (src_x>=0 && src_x<width && src_y>=0 && src_y<height) {
                    src_x -= 0.5; // 0
                    src_y -= 0.5; // 0 to 4
                    Colour<ch,ach> c00 = this->pixelSafe(floorf(src_x+0), floorf(src_y+0), bg);
                    Colour<ch,ach> c01 = this->pixelSafe(floorf(src_x+1), floorf(src_y+0), bg);
                    Colour<ch,ach> c10 = this->pixelSafe(floorf(src_x+0), floorf(src_y+1), bg);
                    Colour<ch,ach> c11 = this->pixelSafe(floorf(src_x+1), floorf(src_y+1), bg);
                    float frac_x = src_x - floorf(src_x);
                    float frac_y = src_y - floorf(src_y);
                    Colour<ch,ach> c0x = colour_lerp(c00, c01, frac_x);
                    Colour<ch,ach> c1x = colour_lerp(c10, c11, frac_x);
                    ret->pixel(x,y) = colour_lerp(c0x, c1x, frac_y);
                } else {
                    ret->pixel(x,y) = bg;
                }
            }
        }
        return ret;
    }

    Image<ch, ach> *clone (bool flip_x, bool flip_y) const
    {
        Image<ch, ach> *ret = new Image<ch, ach>(width, height);
        if (flip_x) {
            if (flip_y) {
                for (uimglen_t y=0 ; y<height ; ++y)
                    for (uimglen_t x=0 ; x<width ; ++x)
                        ret->pixel(x,y) = this->pixel(width-x-1, height-y-1);
            } else {
                for (uimglen_t y=0 ; y<height ; ++y)
                    for (uimglen_t x=0 ; x<width ; ++x)
                        ret->pixel(x,y) = this->pixel(width-x-1, y);
            }
        } else {
            if (flip_y) {
                for (uimglen_t y=0 ; y<height ; ++y)
                    for (uimglen_t x=0 ; x<width ; ++x)
                        ret->pixel(x,y) = this->pixel(x, height-y-1);
            } else {
                for (uimglen_t y=0 ; y<height ; ++y)
                    for (uimglen_t x=0 ; x<width ; ++x)
                        ret->pixel(x,y) = this->pixel(x, y);
            }
        }
        return ret;
    }

    Image<ch,ach> *normalise (void) const
    {
        Colour<ch,ach> pos_total(0.0f);
        Colour<ch,ach> neg_total(0.0f);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                for (chan_t c=0 ; c<ch+ach ; ++c) {
                    float v = this->pixel(x,y)[c];
                    if (v >= 0) {
                        pos_total[c] += v;
                    } else {
                        neg_total[c] -= v;
                    }
                }
            }
        }
        Image<ch,ach> *ret = new Image<ch,ach>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                Colour<ch,ach> norm_pix(0.0f);
                for (chan_t c=0 ; c<ch+ach ; ++c) {
                    float v = this->pixel(x,y)[c];
                    if (v >= 0) {
                        ret->pixel(x,y)[c] = v / pos_total[c];
                    } else {
                        ret->pixel(x,y)[c] = v / neg_total[c];
                    }
                }
            }
        }
        return ret;
    }

    Image<ch,ach> *scale (uimglen_t w, uimglen_t h, ScaleFilter filter) const
    {
        return static_cast<Image<ch,ach>*>(ImageBase::scale(w,h, filter));
    }

    void drawImage (const ImageBase *src_, simglen_t left, simglen_t bottom)
    {
        const Image<ch,1> *src = static_cast<const Image<ch,1>*>(src_);
        uimglen_t w = src->width;
        uimglen_t h = src->height;

        // ignore pixels that go off the left of the image
        simglen_t crop_left = left<0 ? -left : 0;
        simglen_t crop_bottom = bottom<0 ? -bottom : 0;

        // ignore pixels that go off the top or right
        if (left+w > width) w = width - left;
        if (bottom+h > height) h = height - bottom;

        for (uimglen_t y=crop_bottom ; y<h ; ++y) {
            for (uimglen_t x=crop_left ; x<w ; ++x) {
                this->pixel(x+left,y+bottom) = colour_blend(src->pixel(x,y), this->pixel(x+left,y+bottom));
            }
        }
    }

    Image<ch,ach> *convolve (const Image<1,0> *kernel, bool wrap_x, bool wrap_y) const
    {
        // TODO: optimisations
        // 1) use a separate loop for the middle of the image where we don't need to test for edges
        simglen_t kcx = kernel->width / 2;
        simglen_t kcy = kernel->height / 2;
        Image<ch,ach> *ret = new Image<ch,ach>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                Colour<ch,ach> p(0);
                for (simglen_t ky=-kcy ; ky<=kcy ; ++ky) {
                    for (simglen_t kx=-kcx ; kx<=kcx ; ++kx) {
                        float kv = kernel->pixel(kx+kcx, ky+kcy)[0];
                        simglen_t this_x = x+kx;
                        simglen_t this_y = y+ky;
                        if (this_x < 0) this_x = wrap_x ? mymod(this_x, width): 0;
                        if (this_y < 0) this_y = wrap_y ? mymod(this_y, height): 0;
                        if (uimglen_t(this_x) >= width) this_x = wrap_x ? mymod(this_x, width): width-1;
                        if (uimglen_t(this_y) >= height) this_y = wrap_y ? mymod(this_y, height): height-1;
                        Colour<ch,ach> thisv = this->pixel((uimglen_t)this_x, (uimglen_t)this_y);
                        for (chan_t c=0 ; c<ch+ach ; ++c) {
                            p[c] += thisv[c] * kv;
                        }
                    }
                }
                ret->pixel(x,y) = p;
            }
        }
        return ret;
    }

};

static inline uimglen_t get_width (const ImageBase *a, const ColourBase *) { return a->width; }
static inline uimglen_t get_height (const ImageBase *a, const ColourBase *) { return a->height; }
static inline uimglen_t get_width (const ColourBase *, const ImageBase *b) { return b->width; }
static inline uimglen_t get_height (const ColourBase *, const ImageBase *b) { return b->height; }
static inline uimglen_t get_width (const ImageBase *, const ImageBase *b) { return b->width; }
static inline uimglen_t get_height (const ImageBase *, const ImageBase *b) { return b->height; }

// TA and TB can be Image<ch,_> or Colour<ch,_>
// must be compatible except for alpha channels
template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float op(float,float), class T1, class T2> 
Image<ch2,ach2> *image_zip_regular (T1 a, T2 b)
{
    if (ch1 != ch2) abort();
    uimglen_t width = get_width(a,b);
    uimglen_t height = get_height(a,b);
    Image<ch2,ach2> *ret = new Image<ch2,ach2>(width, height);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            ret->pixel(x, y) = colour_zip<ch1,ach1,ch2,ach2,op>(a->pixel(x, y), b->pixel(x,y));
        }
    }
    return ret;
}

// TA can be Image<1,0> or Colour<1,0>
template<chan_t ch2, chan_t ach2, float op(float,float), class T1, class T2> 
Image<ch2,ach2> *image_zip_left_mask (T1 a, T2 b)
{
    uimglen_t width = get_width(a,b);
    uimglen_t height = get_height(a,b);
    Image<ch2,ach2> *ret = new Image<ch2,ach2>(width, height);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            ret->pixel(x, y) = colour_zip<ch2,0,ch2,ach2,op>(Colour<ch2,0>(a->pixel(x, y)[0]), b->pixel(x,y));
        }
    }
    return ret;
}

// TB can be Image<1,0> or Colour<1,0>
template<chan_t ch1, chan_t ach1, float op(float,float), class T1, class T2> 
Image<ch1,0> *image_zip_right_mask (T1 a, T2 b)
{
    uimglen_t width = get_width(a,b);
    uimglen_t height = get_height(a,b);
    Image<ch1,0> *ret = new Image<ch1,0>(width, height);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            ret->pixel(x, y) = colour_zip<ch1,ach1,ch1,0,op>(a->pixel(x,y), Colour<ch1,0>(b->pixel(x, y)[0]));
        }
    }
    return ret;
}


// TA and TB can be Image<ch,_> or Colour<ch,_>
// must be compatible except for alpha channels
template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, class T1, class T2> 
Image<ch2,ach2> *image_blend_regular (T1 a, T2 b)
{
    if (ch1 != ch2) abort();
    uimglen_t width = get_width(a,b);
    uimglen_t height = get_height(a,b);
    Image<ch2,ach2> *ret = new Image<ch2,ach2>(width, height);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            ret->pixel(x, y) = colour_blend<ch1,ach1,ch2,ach2>(a->pixel(x, y), b->pixel(x,y));
        }
    }
    return ret;
}

// TA can be Image<1,0> or Colour<1,0>
template<chan_t ch2, chan_t ach2, class T1, class T2> 
Image<ch2,ach2> *image_blend_left_mask (T1 a, T2 b)
{
    uimglen_t width = get_width(a,b);
    uimglen_t height = get_height(a,b);
    Image<ch2,ach2> *ret = new Image<ch2,ach2>(width, height);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            ret->pixel(x, y) = colour_blend<ch2,0,ch2,ach2>(Colour<ch2,0>(a->pixel(x, y)[0]), b->pixel(x,y));
        }
    }
    return ret;
}

// TB can be Image<1,0> or Colour<1,0>
template<chan_t ch1, chan_t ach1, class T1, class T2> 
Image<ch1,0> *image_blend_right_mask (T1 a, T2 b)
{
    uimglen_t width = get_width(a,b);
    uimglen_t height = get_height(a,b);
    Image<ch1,0> *ret = new Image<ch1,0>(width, height);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            ret->pixel(x, y) = colour_blend<ch1,ach1,ch1,0>(a->pixel(x,y), Colour<ch1,0>(b->pixel(x, y)[0]));
        }
    }
    return ret;
}


// TA and TB can be Image<ch,ach> or Colour<ch,ach>
template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float zop(float,float), float rop(float,float), class T1, class T2> 
ColourBase *image_zip_reduce_regular (T1 a, T2 b)
{
    if (ch1 != ch2) abort();
    if (ach1 != ach2) abort();
    uimglen_t width = get_width(a,b);
    uimglen_t height = get_height(a,b);
    Colour<ch1,ach1> *r = new Colour<ch1,ach1>(0);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            Colour<ch1,ach1> ac = a->pixel(x,y);
            Colour<ch2,ach2> bc = b->pixel(x,y);
            for (chan_t c=0 ; c<ch1+ach1 ; ++c) {
                float zr = zop(ac[c], bc[c]);
                (*r)[c] = rop((*r)[c], zr);
            }
        }
    }
    return r;
}

// TA can be Image<1,0> or Colour<1,0>
template<chan_t ch, chan_t ach, float zop(float,float), float rop(float,float), class T1, class T2> 
ColourBase *image_zip_reduce_left_mask (T1 a, T2 b)
{
    uimglen_t width = get_width(a,b);
    uimglen_t height = get_height(a,b);
    Colour<ch,ach> *r = new Colour<ch,ach>(0);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            Colour<ch,ach> ac = Colour<ch,ach>(a->pixel(x,y)[0]);
            Colour<ch,ach> bc = b->pixel(x,y);
            for (chan_t c=0 ; c<ch+ach ; ++c) {
                float zr = zop(ac[c], bc[c]);
                (*r)[c] = rop((*r)[c], zr);
            }
        }
    }
    return r;
}

// TB can be Image<1,0> or Colour<1,0>
template<chan_t ch, chan_t ach, float zop(float,float), float rop(float,float), class T1, class T2> 
ColourBase *image_zip_reduce_right_mask (T1 a, T2 b)
{
    uimglen_t width = get_width(a,b);
    uimglen_t height = get_height(a,b);
    Colour<ch,ach> *r = new Colour<ch,ach>(0);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            Colour<ch,ach> ac = a->pixel(x,y);
            Colour<ch,ach> bc = Colour<ch,ach>(b->pixel(x,y)[0]);
            for (chan_t c=0 ; c<ch+ach ; ++c) {
                float zr = zop(ac[c], bc[c]);
                (*r)[c] = rop((*r)[c], zr);
            }
        }
    }
    return r;
}


// TA and TB can be Image<ch,_> or Colour<ch,_>
// must be compatible except for alpha channels
template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, class T1, class T2> 
Image<ch2,ach2> *global_lerp_regular (T1 a, T2 b, float param)
{
    if (ch1 != ch2) abort();
    uimglen_t width = get_width(a,b);
    uimglen_t height = get_height(a,b);
    Image<ch2,ach2> *ret = new Image<ch2,ach2>(width, height);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            ret->pixel(x, y) = colour_lerp<ch1,ach1,ch2,ach2>(a->pixel(x, y), b->pixel(x,y), param);
        }
    }
    return ret;
}

// TA can be Image<1,0> or Colour<1,0>
template<chan_t ch2, chan_t ach2, class T1, class T2> 
Image<ch2,ach2> *global_lerp_left_mask (T1 a, T2 b, float param)
{
    uimglen_t width = get_width(a,b);
    uimglen_t height = get_height(a,b);
    Image<ch2,ach2> *ret = new Image<ch2,ach2>(width, height);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            ret->pixel(x, y) = colour_lerp<ch2,0,ch2,ach2>(Colour<ch2,0>(a->pixel(x, y)[0]), b->pixel(x,y), param);
        }
    }
    return ret;
}

// TB can be Image<1,0> or Colour<1,0>
template<chan_t ch1, chan_t ach1, class T1, class T2> 
Image<ch1,ach1> *global_lerp_right_mask (T1 a, T2 b, float param)
{
    uimglen_t width = get_width(a,b);
    uimglen_t height = get_height(a,b);
    Image<ch1,ach1> *ret = new Image<ch1,ach1>(width, height);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            ret->pixel(x, y) = colour_lerp<ch1,ach1,ch1,ach1>(a->pixel(x,y), Colour<ch1,ach1>(b->pixel(x, y)[0]), param);
        }
    }
    return ret;
}


// must be compatible
template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
Colour<ch2,ach2> global_lerp_colour_regular (const Colour<ch1,ach1> *a, const Colour<ch2,ach2> *b, float param)
{
    if (ach1 != ach2) abort();
    if (ch1 != ch2) abort();
    return colour_lerp<ch1,ach1,ch2,ach2>(*a, *b, param);
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
Colour<ch2,ach2> global_lerp_colour_left_mask (const Colour<ch1,ach1> *a, const Colour<ch2,ach2> *b, float param)
{
    if (ach1 != 1) abort();
    if (ch1 != 0) abort();
    return colour_lerp<ch2,ach2,ch2,ach2>(Colour<ch2,ach2>((*a)[0]), *b, param);
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
Colour<ch1,ach1> global_lerp_colour_right_mask (const Colour<ch1,ach1> *a, const Colour<ch2,ach2> *b, float param)
{
    if (ach2 != 1) abort();
    if (ch2 != 0) abort();
    return colour_lerp<ch1,ach1,ch1,ach1>(*a, Colour<ch1,ach1>((*b)[0]), param);
}



ImageBase *image_load (const std::string &filename);

bool image_save (ImageBase *image, const std::string &filename);

template<chan_t ch, chan_t ach> ImageBase *image_make (uimglen_t width, uimglen_t height, const ColourBase &init_)
{
    Image<ch,ach> *my_image = new Image<ch,ach>(width, height);
    const Colour<ch,ach> &init = static_cast<const Colour<ch,ach>&>(init_);

    for (uimglen_t y=0 ; y<my_image->height ; ++y) {
        for (uimglen_t x=0 ; x<my_image->width ; ++x) {
            my_image->pixel(x, y) = init;
        }
    }

    return my_image;
}

#endif
