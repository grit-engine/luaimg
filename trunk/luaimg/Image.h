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
struct PixelBase;
template<chan_t ch> struct Pixel;
class ImageBase;
template<chan_t ch> class Image;

void RGBtoHSL (float R, float G, float B, float &H, float &S, float &L);
void HSLtoRGB (float H, float S, float L, float &R, float &G, float &B);
void HSLtoHSV (float HH, float SS, float LL, float &H, float &S, float &L);
void HSVtoHSL (float H, float S, float L, float &HH, float &SS, float &LL);
void RGBtoHSV (float R, float G, float B, float &H, float &S, float &L);
void HSVtoRGB (float H, float S, float L, float &R, float &G, float &B);

#ifndef IMAGE_H
#define IMAGE_H

#include <cmath>

#include <ostream>
#include <string>

enum ScaleFilter {
    SF_BOX,
    SF_BILINEAR,
    SF_BSPLINE,
    SF_BICUBIC,
    SF_CATMULLROM,
    SF_LANCZOS3
};

struct PixelBase {
    virtual chan_t channels() const = 0;
    virtual float *raw() = 0;
    virtual ~PixelBase (void) { }
};

/* Main purpose of this is to avoid C/C++ stupid array type syntax. */
template<chan_t ch> struct Pixel : PixelBase {
    Pixel () { }
    Pixel (float d) { for (chan_t c=0 ; c<ch ; ++c) v[c] = d; }
    Pixel (const Pixel<1> &d) { for (chan_t c=0 ; c<ch ; ++c) v[c] = d[0]; }
    float v[ch];
    chan_t channels() const { return ch; }
    virtual float *raw() { return &(*this)[0]; }
    const float &operator[] (chan_t i) const { return v[i]; }
    float &operator[] (chan_t i) { return v[i]; }

    Pixel<ch> pow (float other) const { Pixel<ch> r; for (chan_t c=0 ; c<ch ; ++c) r[c] = powf((*this)[c],other); return r; }

    Pixel<ch> add (const Pixel<ch> &other) const { Pixel<ch> r; for (chan_t c=0 ; c<ch ; ++c) r[c] = (*this)[c] + other[c]; return r; }
    Pixel<ch> sub (const Pixel<ch> &other) const { Pixel<ch> r; for (chan_t c=0 ; c<ch ; ++c) r[c] = (*this)[c] - other[c]; return r; }
    Pixel<ch> mul (const Pixel<ch> &other) const { Pixel<ch> r; for (chan_t c=0 ; c<ch ; ++c) r[c] = (*this)[c] * other[c]; return r; }
    Pixel<ch> div (const Pixel<ch> &other) const { Pixel<ch> r; for (chan_t c=0 ; c<ch ; ++c) r[c] = (*this)[c] / other[c]; return r; }
    Pixel<ch> min (const Pixel<ch> &other) const { Pixel<ch> r; for (chan_t c=0 ; c<ch ; ++c) r[c] = std::min((*this)[c], other[c]); return r; }
    Pixel<ch> max (const Pixel<ch> &other) const { Pixel<ch> r; for (chan_t c=0 ; c<ch ; ++c) r[c] = std::max((*this)[c], other[c]); return r; }

    Pixel<ch> lerp (const Pixel<ch> &other, float alpha) const {
        Pixel<ch> r;
        for (chan_t c=0 ; c<ch ; ++c) {
            r[c] = (1-alpha)*(*this)[c] + alpha*other[c];
        }
        return r;
    }

    Pixel<ch> unm (void) const { Pixel<ch> r; for (chan_t c=0 ; c<ch ; ++c) r[c] = -(*this)[c]; return r; }

    Pixel<ch> abs (void) const { Pixel<ch> r; for (chan_t c=0 ; c<ch ; ++c) r[c] = fabsf((*this)[c]); return r; }

    Pixel<ch> alphaBlendNoDestAlpha (const Pixel<ch+1> &other) const
    {
        float alpha = std::max(0.0f, std::min(1.0f, other[ch]));
        Pixel<ch> r;
        for (chan_t c=0 ; c<ch ; ++c) {
            r[c] = (1-alpha)*(*this)[c] + alpha*other[c];
        }
        return r;
    }

    Pixel<ch> alphaBlend (const Pixel<ch> &other) const
    {
        float alpha = std::max(0.0f, std::min(1.0f, other[ch-1]));
        float old_alpha = std::max(0.0f, std::min(1.0f, (*this)[ch-1]));
        float new_alpha = 1 - (1-alpha)*(1-old_alpha);
        Pixel<ch> r;
        for (chan_t c=0 ; c<ch-1 ; ++c) {
            r[c] = (1-alpha/new_alpha)*(*this)[c] + alpha/new_alpha*other[c];
        }
        r[ch-1] = new_alpha;
        return r;
    }
};

static inline std::ostream &operator<<(std::ostream &o, const Pixel<3> &p)
{
    o << "(" << p[0] << ", " << p[1] << ", " << p[2] << ")";
    return o;
}


class ImageBase {

    public:

    const uimglen_t width, height;
    ImageBase (uimglen_t width, uimglen_t height)
      : width(width), height(height)
    {
    }

    unsigned long numPixels() { return (unsigned long)(height) * width; };

    virtual ~ImageBase (void) { }

    bool sizeCompatibleWith (const ImageBase *other) const {
        if (other->width != width) return false;
        if (other->height != height) return false;
        return true;
    }

    bool compatibleWith (const ImageBase *other) const {
        if (!sizeCompatibleWith(other)) return false;
        if (other->channels() != channels()) return false;
        return true;
    }

    virtual PixelBase &pixelSlow (uimglen_t x, uimglen_t y) = 0;
    virtual const PixelBase &pixelSlow (uimglen_t x, uimglen_t y) const = 0;

    virtual chan_t channels (void) const = 0;

    virtual float rms (const ImageBase *other) const = 0;

    virtual ImageBase *pow (float other) const = 0;

    virtual ImageBase *add (const ImageBase *other) const = 0;
    virtual ImageBase *sub (const ImageBase *other) const = 0;
    virtual ImageBase *mul (const ImageBase *other) const = 0;
    virtual ImageBase *div (const ImageBase *other) const = 0;
    virtual ImageBase *min (const ImageBase *other) const = 0;
    virtual ImageBase *max (const ImageBase *other) const = 0;
    virtual ImageBase *lerp (const ImageBase *other, float alpha) const = 0;

    virtual ImageBase *blendColourNoDestAlphaSwapped (const PixelBase &other) const = 0;
    virtual ImageBase *blendColourSwapped (const PixelBase &other) const = 0;
    virtual ImageBase *blendColourNoDestAlpha (const PixelBase &other) const = 0;
    virtual ImageBase *blendColour (const PixelBase &other) const = 0;
    virtual ImageBase *blendImageNoDestAlpha (const ImageBase *other) const = 0;
    virtual ImageBase *blendImage (const ImageBase *other) const = 0;

    virtual ImageBase *unm (void) const = 0;
    virtual ImageBase *abs (void) const = 0;

    virtual ImageBase *clone (bool flip_x, bool flip_y) const = 0;
    virtual ImageBase *normalise (void) const = 0;

    virtual ImageBase *scale (uimglen_t width, uimglen_t height, ScaleFilter filter) const;
    virtual ImageBase *rotate (float angle) const;

    virtual ImageBase *convolve (const Image<1> *kernel, bool wrap_x, bool wrap_y) const = 0;

};

static inline std::ostream &operator<<(std::ostream &o, ImageBase &img)
{
    o << "Image ("<<img.width<<","<<img.height<<")x"<<int(img.channels())<<" [0x"<<&img<<"]";
    return o;
}

/* Avoid recursive template instantiation -- break the cycle at 0 */
template<chan_t ch> 
Image<ch-1> *blend_colour_no_dest_alpha_swapped (const Image<ch> *icing, const PixelBase &colour_);

template<> 
Image<0> *blend_colour_no_dest_alpha_swapped<1> (const Image<1> *icing, const PixelBase &colour_);

template<chan_t ch> class Image : public ImageBase {

    Pixel<ch> * data;

    public:

    Image (uimglen_t width, uimglen_t height)
      : ImageBase(width, height)
    {
        data = new Pixel<ch>[numPixels()];
    }

    ~Image (void)
    {
        delete [] data;
    }

    Pixel<ch> &pixel (uimglen_t x, uimglen_t y) { return data[y*width+x]; }
    const Pixel<ch> &pixel (uimglen_t x, uimglen_t y) const { return data[y*width+x]; }

    Pixel<ch> &pixelSlow (uimglen_t x, uimglen_t y) { return pixel(x,y); }
    const Pixel<ch> &pixelSlow (uimglen_t x, uimglen_t y) const { return pixel(x,y); }

    chan_t channels (void) const { return ch; }

    float rmsMask (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<1>*>(other_);
        float ret = 0;
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                for (chan_t c=0 ; c<ch ; ++c) {
                    float diff = this->pixel(x,y)[c] - other->pixel(x,y)[0];
                    ret += diff * diff;
                }
            }
        }
        return ret;
    }

    float rms (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<ch>*>(other_);
        float ret = 0;
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                for (chan_t c=0 ; c<ch ; ++c) {
                    float diff = this->pixel(x,y)[c] - other->pixel(x,y)[c];
                    ret += diff * diff;
                }
            }
        }
        return ret;
    }

    Image<ch> *pow (float other) const
    {
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x,y) = this->pixel(x,y).pow(other);
            }
        }
        return ret;
    }

    Image<ch> *addMask (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<1>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).add(other->pixel(x, y));
            }
        }
        return ret;
    }
    Image<ch> *subMask (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<1>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).sub(other->pixel(x, y));
            }
        }
        return ret;
    }
    Image<ch> *mulMask (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<1>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).mul(other->pixel(x, y));
            }
        }
        return ret;
    }
    Image<ch> *divMask (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<1>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).div(other->pixel(x, y));
            }
        }
        return ret;
    }
    Image<ch> *maxMask (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<1>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).max(other->pixel(x, y));
            }
        }
        return ret;
    }
    Image<ch> *minMask (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<1>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).min(other->pixel(x, y));
            }
        }
        return ret;
    }
    Image<ch> *lerpMask (const ImageBase *other_, float alpha) const
    {
        const Image<ch> *other = static_cast<const Image<1>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).lerp(other->pixel(x, y), alpha);
            }
        }
        return ret;
    }


    Image<ch> *add (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<ch>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).add(other->pixel(x, y));
            }
        }
        return ret;
    }
    Image<ch> *sub (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<ch>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).sub(other->pixel(x, y));
            }
        }
        return ret;
    }
    Image<ch> *mul (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<ch>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).mul(other->pixel(x, y));
            }
        }
        return ret;
    }
    Image<ch> *div (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<ch>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).div(other->pixel(x, y));
            }
        }
        return ret;
    }
    Image<ch> *max (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<ch>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).max(other->pixel(x, y));
            }
        }
        return ret;
    }
    Image<ch> *min (const ImageBase *other_) const
    {
        const Image<ch> *other = static_cast<const Image<ch>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).min(other->pixel(x, y));
            }
        }
        return ret;
    }
    Image<ch> *lerp (const ImageBase *other_, float alpha) const
    {
        const Image<ch> *other = static_cast<const Image<ch>*>(other_);
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).lerp(other->pixel(x, y), alpha);
            }
        }
        return ret;
    }


    Image<ch> *unm (void) const
    {
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x,y) = this->pixel(x,y).unm();
            }
        }
        return ret;
    }

    Image<ch> *abs (void) const
    {
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x,y) = this->pixel(x,y).abs();
            }
        }
        return ret;
    }

    Image<ch> *add (const Pixel<ch> &p) const
    {
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).add(p);
            }
        }
        return ret;
    }
    Image<ch> *sub (const Pixel<ch> &p, bool swapped) const
    {
        Image<ch> *ret = new Image<ch>(width, height);
        if (swapped) {
            for (uimglen_t y=0 ; y<height ; ++y) {
                for (uimglen_t x=0 ; x<width ; ++x) {
                    ret->pixel(x, y) = p.sub(this->pixel(x, y));
                }
            }
        } else {
            for (uimglen_t y=0 ; y<height ; ++y) {
                for (uimglen_t x=0 ; x<width ; ++x) {
                    ret->pixel(x, y) = this->pixel(x, y).sub(p);
                }
            }
        }
        return ret;
    }
    Image<ch> *mul (const Pixel<ch> &p) const
    {
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).mul(p);
            }
        }
        return ret;
    }
    Image<ch> *div (const Pixel<ch> &p, bool swapped) const
    {
        Image<ch> *ret = new Image<ch>(width, height);
        if (swapped) {
            for (uimglen_t y=0 ; y<height ; ++y) {
                for (uimglen_t x=0 ; x<width ; ++x) {
                    ret->pixel(x, y) = p.div(this->pixel(x, y));
                }
            }
        } else {
            for (uimglen_t y=0 ; y<height ; ++y) {
                for (uimglen_t x=0 ; x<width ; ++x) {
                    ret->pixel(x, y) = this->pixel(x, y).div(p);
                }
            }
        }
        return ret;
    }
    Image<ch> *max (const Pixel<ch> &p) const
    {
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).max(p);
            }
        }
        return ret;
    }
    Image<ch> *min (const Pixel<ch> &p) const
    {
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).min(p);
            }
        }
        return ret;
    }
    Image<ch> *lerp (const Pixel<ch> &p, float alpha) const
    {
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                ret->pixel(x, y) = this->pixel(x, y).lerp(p, alpha);
            }
        }
        return ret;
    }


    Image<ch> *crop (simglen_t left, simglen_t bottom, uimglen_t w, uimglen_t h, const Pixel<ch> &zero) const
    {
        Image<ch> *ret = new Image<ch>(w, h);
        for (uimglen_t y=0 ; y<h ; ++y) {
            for (uimglen_t x=0 ; x<w ; ++x) {
                uimglen_t old_x = x+left;
                uimglen_t old_y = y+bottom;
                if (old_x < width && old_y < height) {
                    ret->pixel(x,y) = this->pixel(old_x, old_y);
                } else {
                    ret->pixel(x,y) = zero;
                }
            }
        }
        return ret;
    }

    Image<ch> *clone (bool flip_x, bool flip_y) const
    {
        Image<ch> *ret = new Image<ch>(width, height);
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

    Image<ch> *normalise (void) const
    {
        Pixel<ch> pos_total(0.0f);
        Pixel<ch> neg_total(0.0f);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                for (chan_t c=0 ; c<ch ; ++c) {
                    float v = this->pixel(x,y)[c];
                    if (v >= 0) {
                        pos_total[c] += v;
                    } else {
                        neg_total[c] -= v;
                    }
                }
            }
        }
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                Pixel<ch> norm_pix(0.0f);
                for (chan_t c=0 ; c<ch ; ++c) {
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

    Image<ch> *scale (uimglen_t w, uimglen_t h, ScaleFilter filter) const
    {
        return static_cast<Image<ch>*>(ImageBase::scale(w,h, filter));
    }

    Image<ch> *rotate (float angle) const
    {
        return static_cast<Image<ch>*>(ImageBase::rotate(angle));
    }

    Image<ch-1> *blendColourNoDestAlphaSwapped (const PixelBase &colour_) const
    {
        return blend_colour_no_dest_alpha_swapped(this, colour_);
    }

    Image<ch> *blendColourNoDestAlpha (const PixelBase &colour_) const
    {
        const Pixel<ch+1> &colour = static_cast<const Pixel<ch+1>&>(colour_);
        Image<ch> *r = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                r->pixel(x,y) = this->pixel(x,y).alphaBlendNoDestAlpha(colour);
            }
        }
        return r;
    }

    Image<ch> *blendColourSwapped (const PixelBase &colour_) const
    {
        const Pixel<ch> &colour = static_cast<const Pixel<ch>&>(colour_);
        Image<ch> *r = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                r->pixel(x,y) = colour.alphaBlend(this->pixel(x,y));
            }
        }
        return r;
    }

    Image<ch> *blendColour (const PixelBase &colour_) const
    {
        const Pixel<ch> &colour = static_cast<const Pixel<ch>&>(colour_);
        Image<ch> *r = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                r->pixel(x,y) = this->pixel(x,y).alphaBlend(colour);
            }
        }
        return r;
    }

    Image<ch> *blendImageNoDestAlpha (const ImageBase *src_) const
    {
        const Image<ch+1> *src = static_cast<const Image<ch+1>*>(src_);
        Image<ch> *r = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                r->pixel(x,y) = this->pixel(x,y).alphaBlendNoDestAlpha(src->pixel(x,y));
            }
        }
        return r;
    }

    Image<ch> *blendImage (const ImageBase *src_) const
    {
        const Image<ch> *src = static_cast<const Image<ch>*>(src_);
        Image<ch> *r = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                r->pixel(x,y) = this->pixel(x,y).alphaBlend(src->pixel(x,y));
            }
        }
        return r;
    }

    void drawImageNoDestAlpha (const ImageBase *src_, uimglen_t left, uimglen_t bottom)
    {
        const Image<ch+1> *src = static_cast<const Image<ch+1>*>(src_);
        uimglen_t w = src->width;
        uimglen_t h = src->height;
        if (left+w >= width) w = width - left - 1;
        if (bottom+h >= height) h = height - bottom - 1;
        for (uimglen_t y=0 ; y<h ; ++y) {
            for (uimglen_t x=0 ; x<w ; ++x) {
                this->pixel(x+left,y+bottom) = this->pixel(x+left,y+bottom).alphaBlendNoDestAlpha(src->pixel(x,y));
            }
        }
    }

    void drawImage (const ImageBase *src_, uimglen_t left, uimglen_t bottom)
    {
        const Image<ch> *src = static_cast<const Image<ch>*>(src_);
        uimglen_t w = src->width;
        uimglen_t h = src->height;
        if (left+w >= width) w = width - left - 1;
        if (bottom+h >= height) h = height - bottom - 1;
        for (uimglen_t y=0 ; y<h ; ++y) {
            for (uimglen_t x=0 ; x<w ; ++x) {
                this->pixel(x+left,y+bottom) = this->pixel(x+left,y+bottom).alphaBlend(src->pixel(x,y));
            }
        }
    }

    Image<ch> *convolve (const Image<1> *kernel, bool wrap_x, bool wrap_y) const
    {
        // TODO: several obvious optimisations should be implemented here
        simglen_t kcx = kernel->width / 2;
        simglen_t kcy = kernel->height / 2;
        Image<ch> *ret = new Image<ch>(width, height);
        for (uimglen_t y=0 ; y<height ; ++y) {
            for (uimglen_t x=0 ; x<width ; ++x) {
                Pixel<ch> p(0);
                for (simglen_t ky=-kcy ; ky<=kcy ; ++ky) {
                    for (simglen_t kx=-kcx ; kx<=kcx ; ++kx) {
                        float kv = kernel->pixel(kx+kcx, ky+kcy)[0];
                        simglen_t this_x = x+kx;
                        simglen_t this_y = y+ky;
                        while (this_x < 0) this_x = wrap_x ? this_x + width: 0;
                        while (this_y < 0) this_y = wrap_y ? this_y + height: 0;
                        while (this_x >= (simglen_t)width) this_x = wrap_x ? this_x - width: width-1;
                        while (this_y >= (simglen_t)height) this_y = wrap_y ? this_y - height: height-1;
                        Pixel<ch> thisv = this->pixel((uimglen_t)this_x, (uimglen_t)this_y);
                        p = thisv.mul(kv).add(p);

                    }
                }
                ret->pixel(x,y) = p;
            }
        }
        return ret;
    }

};

template<chan_t ch> 
inline Image<ch-1> *blend_colour_no_dest_alpha_swapped (const Image<ch> *icing, const PixelBase &colour_)
{
    const Pixel<ch-1> &colour = static_cast<const Pixel<ch-1>&>(colour_);
    Image<ch-1> *r = new Image<ch-1>(icing->width, icing->height);
    for (uimglen_t y=0 ; y<icing->height ; ++y) {
        for (uimglen_t x=0 ; x<icing->width ; ++x) {
            r->pixel(x,y) = colour.alphaBlendNoDestAlpha(icing->pixel(x,y));
        }
    }
    return r;
}
template<> 
inline Image<0> *blend_colour_no_dest_alpha_swapped<1> (const Image<1> *, const PixelBase &)
{
    return NULL;
}

ImageBase *image_load (const std::string &filename);

bool image_save (ImageBase *image, const std::string &filename);

template<chan_t ch> Image<ch> *image_make (uimglen_t width, uimglen_t height, float (&init)[ch])
{
    Image<ch> *my_image = new Image<ch>(width, height);

    for (uimglen_t y=0 ; y<my_image->height ; ++y) {
        for (uimglen_t x=0 ; x<my_image->width ; ++x) {
            for (chan_t c=0 ; c<ch ; ++c) {
                my_image->pixel(x, y)[c] = init[c];
            }
        }
    }

    return my_image;
    
}

template<chan_t ch> Image<ch> *image_make (uimglen_t width, uimglen_t height, const Pixel<ch> &zero)
{
    Image<ch> *my_image = new Image<ch>(width, height);

    for (uimglen_t y=0 ; y<my_image->height ; ++y) {
        for (uimglen_t x=0 ; x<my_image->width ; ++x) {
            my_image->pixel(x, y) = zero;
        }
    }

    return my_image;
    
}

template<chan_t ch> Image<ch> *image_make (uimglen_t width, uimglen_t height)
{
    Image<ch> *my_image = new Image<ch>(width, height);

    for (uimglen_t y=0 ; y<my_image->height ; ++y) {
        for (uimglen_t x=0 ; x<my_image->width ; ++x) {
            for (chan_t c=0 ; c<ch ; ++c) {
                my_image->pixel(x, y)[c] = 0.0f;
            }
        }
    }

    return my_image;
    
}

#endif
