/* Copyright (c) David Cunningham and the Grit Game Engine project 2014
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

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <limits>
#include <algorithm>

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

#include <lua_util.h>
#include <unicode_util.h>
#include <sleep.h>
#include <exception.h>

#include "lua_wrappers_image.h"

#include "Image.h"
#include "text.h"
//#include "VoxelImage.h"


static void check_scoord (lua_State *L, int index, simglen_t &x, simglen_t &y)
{
    float x_, y_;
    lua_checkvector2(L, index, &x_, &y_);
    x = x_ < std::numeric_limits<simglen_t>::min() ? std::numeric_limits<simglen_t>::min()
      : x_ > std::numeric_limits<simglen_t>::max() ? std::numeric_limits<simglen_t>::max() : x_;
    y = y_ < std::numeric_limits<simglen_t>::min() ? std::numeric_limits<simglen_t>::min()
      : y_ > std::numeric_limits<simglen_t>::max() ? std::numeric_limits<simglen_t>::max() : y_;
}

static void check_coord (lua_State *L, int index, uimglen_t &x, uimglen_t &y)
{
    float x_, y_;
    lua_checkvector2(L, index, &x_, &y_);
    x = x_ < 0 ? 0 : x_ > std::numeric_limits<uimglen_t>::max() ? std::numeric_limits<uimglen_t>::max() : x_;
    y = y_ < 0 ? 0 : y_ > std::numeric_limits<uimglen_t>::max() ? std::numeric_limits<uimglen_t>::max() : y_;
}

chan_t get_colour_channels (lua_State *L, int index)
{
    switch (lua_type(L, index)) {
        case LUA_TNUMBER: return 1;
        case LUA_TVECTOR2: return 2;
        case LUA_TVECTOR3: return 3;
        case LUA_TVECTOR4: return 4;
        default: return 0;
    }
}

template<chan_t ch, chan_t ach> bool check_colour (lua_State *L, Colour<ch, ach> &p, int index);

template<> bool check_colour<1,0> (lua_State *L, Colour<1,0> &p, int index)
{
    if (lua_type(L,index) != LUA_TNUMBER) return false;
    p[0] = lua_tonumber(L, index);
    return true;
}

template<> bool check_colour<2,0> (lua_State *L, Colour<2,0> &p, int index)
{
    if (lua_type(L,index) == LUA_TNUMBER) {
        p[0] = lua_tonumber(L, index);
        p[1] = p[0];
        return true;
    }
    if (lua_type(L,index) != LUA_TVECTOR2) return false;
    lua_checkvector2(L, index, &p[0], &p[1]);
    return true;
}

template<> bool check_colour<1,1> (lua_State *L, Colour<1,1> &p, int index)
{
    if (lua_type(L,index) == LUA_TNUMBER) {
        p[0] = lua_tonumber(L, index);
        p[1] = p[0];
        return true;
    }
    if (lua_type(L,index) != LUA_TVECTOR2) return false;
    lua_checkvector2(L, index, &p[0], &p[1]);
    return true;
}

template<> bool check_colour<3,0> (lua_State *L, Colour<3,0> &p, int index)
{
    if (lua_type(L,index) == LUA_TNUMBER) {
        p[0] = lua_tonumber(L, index);
        p[1] = p[0];
        p[2] = p[0];
        return true;
    }
    if (lua_type(L,index) != LUA_TVECTOR3) return false;
    lua_checkvector3(L, index, &p[0], &p[1], &p[2]);
    return true;
}

template<> bool check_colour<2,1> (lua_State *L, Colour<2,1> &p, int index)
{
    if (lua_type(L,index) == LUA_TNUMBER) {
        p[0] = lua_tonumber(L, index);
        p[1] = p[0];
        p[2] = p[0];
        return true;
    }
    if (lua_type(L,index) != LUA_TVECTOR3) return false;
    lua_checkvector3(L, index, &p[0], &p[1], &p[2]);
    return true;
}

template<> bool check_colour<4,0> (lua_State *L, Colour<4,0> &p, int index)
{
    if (lua_type(L,index) == LUA_TNUMBER) {
        p[0] = lua_tonumber(L, index);
        p[1] = p[0];
        p[2] = p[0];
        p[3] = p[0];
        return true;
    }
    if (lua_type(L,index) != LUA_TVECTOR4) return false;
    lua_checkvector4(L, index, &p[0], &p[1], &p[2], &p[3]);
    return true;
}

template<> bool check_colour<3,1> (lua_State *L, Colour<3,1> &p, int index)
{
    if (lua_type(L,index) == LUA_TNUMBER) {
        p[0] = lua_tonumber(L, index);
        p[1] = p[0];
        p[2] = p[0];
        p[3] = p[0];
        return true;
    }
    if (lua_type(L,index) != LUA_TVECTOR4) return false;
    lua_checkvector4(L, index, &p[0], &p[1], &p[2], &p[3]);
    return true;
}

template<chan_t ch, chan_t ach>
ColourBase *alloc_colour_template (lua_State *L, int index)
{
    // if ach==0 then accepts correct number of channels, or promotes by using number in all dimensions
    // else ach==1 and accepts correct number of channels, or promotes single channel with alpha of 1
    Colour<ch,ach> *r = new Colour<ch,ach>();
    if (ach==1) {
        // try adding alpha value of 1 onto what was given
        Colour<ch,0> r2;
        if (check_colour(L, r2, index)) {
            for (chan_t i=0 ; i<ch ; ++i) (*r)[i] = r2[i];
            (*r)[ch] = 1;
            return r;
        }
    }
    // try an exact match
    if (!check_colour(L, *r, index)) {
        delete r;
        my_lua_error(L, "Expected a "+str(int(ch))+" channel"+(ach==1?" alpha":"")
                        +" colour, got "+type_name(L,index));
        return NULL;
    }
    return r;
}

ColourBase *alloc_colour (lua_State *L, int ch, bool alpha, int index)
{
    switch (ch) {
        case 1: return alloc_colour_template<1,0>(L,index);
        case 2:
        if (alpha) {
            return alloc_colour_template<1,1>(L,index);
        } else {
            return alloc_colour_template<2,0>(L,index);
        }

        case 3:
        if (alpha) {
            return alloc_colour_template<2,1>(L,index);
        } else {
            return alloc_colour_template<3,0>(L,index);
        }

        case 4:
        if (alpha) {
            return alloc_colour_template<3,1>(L,index);
        } else {
            return alloc_colour_template<4,0>(L,index);
        }

        default:
        my_lua_error(L, "Expected a colour at argument "+str(index));
        return NULL;
    }
}

template<chan_t ch, chan_t ach>
void push_colour (lua_State *L, const Colour<ch,ach> &p)
{
    switch (p.channels()) {
        case 1: lua_pushnumber(L, p[0]); break;
        case 2: lua_pushvector2(L, p[0], p[1]); break;
        case 3: lua_pushvector3(L, p[0], p[1], p[2]); break;
        case 4: lua_pushvector4(L, p[0], p[1], p[2], p[3]); break;
        default: my_lua_error(L, "Internal error: weird channels");
    }
}

void push_colour (lua_State *L, chan_t channels, bool has_alpha, const ColourBase &p)
{
    switch (channels) {
        case 1: push_colour(L, static_cast<const Colour<1,0>&>(p));
                break;
        case 2: if (has_alpha) push_colour(L, static_cast<const Colour<1,1>&>(p));
                else           push_colour(L, static_cast<const Colour<2,0>&>(p));
                break;
        case 3: if (has_alpha) push_colour(L, static_cast<const Colour<2,1>&>(p));
                else           push_colour(L, static_cast<const Colour<3,0>&>(p));
                break;
        case 4: if (has_alpha) push_colour(L, static_cast<const Colour<3,1>&>(p));
                else           push_colour(L, static_cast<const Colour<4,0>&>(p));
                break;
        default: my_lua_error(L, "Internal error: weird channels");
    }
}

// image must never have been pushed before, or it will be double-freed upon GC
void push_image (lua_State *L, ImageBase *image)
{
    ASSERT(image != NULL);
    ASSERT(!image->beenPushed);
    image->beenPushed = true;
    void **self_ptr = static_cast<void**>(lua_newuserdata(L, sizeof(*self_ptr)));
    lua_extmemburden(L, image->numBytes());
    *self_ptr = image;
    luaL_getmetatable(L, IMAGE_TAG);
    lua_setmetatable(L, -2);
}


float op_add (float a, float b) { return a+b; }
float op_mul (float a, float b) { return a*b; }
float op_div (float a, float b) { return a/b; }
float op_sub (float a, float b) { return a-b; }
// powf
float op_max (float a, float b) { return a>b?a:b; }
float op_min (float a, float b) { return a<b?a:b; }
float op_diffsq (float a, float b) { return (a-b)*(a-b); }
float op_diff (float a, float b) { return fabsf(a-b); }

// Our job here is to
// 1) call the right image_op function (regular, left_mask, right_mask)
// 2) figure out whether Colour<c,0> or Colour<c-1,1> was intended
// 3) raise an error if ch1/ach1/ch2/ach2 do not work together

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float op(float,float), class T1, class T2>
static ImageBase *image_zip_lua4 (lua_State *L, T1 v1, T2 v2)
{
    // implements (1) and (3) above
    if (ch1 == ch2)
        return image_zip_regular<ch1, ach1, ch2, ach2, op, T1, T2>(v1, v2);
    if (ch1 == 1 && ach1 == 0)
        return image_zip_left_mask<ch2, ach2, op, T1, T2>(v1, v2);
    if (ch2 == 1 && ach2 == 0)
        return image_zip_right_mask<ch1, ach1, op, T1, T2>(v1, v2);
    my_lua_error(L, "Image operation on incompatible images/colours.");
    return NULL;
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float op(float,float)>
static ImageBase *image_zip_lua3 (lua_State *L, const Image<ch1,ach1> *v1, const Image<ch2,ach2> *v2)
{
    // both are images, must check size, but no issue with amibiguous vec(...)
    if (!v1->sizeCompatibleWith(v2)) {
        my_lua_error(L, "Operations require images have the same dimensions.");
    }
    return image_zip_lua4<ch1, ach1, ch2, ach2, op, const Image<ch1,ach1>*, const Image<ch2,ach2>*>(L, v1, v2);
    
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float op(float,float)>
static ImageBase *image_zip_lua3 (lua_State *, const Colour<ch1,ach1> *, const Colour<ch2,ach2> *)
{
    abort();
    return NULL;
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float op(float,float)>
static ImageBase *image_zip_lua3 (lua_State *L, const Image<ch1,ach1> *v1, const Colour<ch2,ach2> *v2)
{
    if (ch1+ach1 == ch2 && ach2 == 0) {
        // redo the colour
        Colour<ch1, ach1> v2_;
        for (chan_t c=0 ; c<ch2 ; ++c) v2_[c] = (*v2)[c];
        return image_zip_lua4<ch1, ach1, ch1, ach1, op, const Image<ch1,ach1>*, const Colour<ch1,ach1>*>(L, v1, &v2_);
    } else {
        return image_zip_lua4<ch1, ach1, ch2, ach2, op, const Image<ch1,ach1>*, const Colour<ch2,ach2>*>(L, v1, v2);
    }
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float op(float,float)>
static ImageBase *image_zip_lua3 (lua_State *L, const Colour<ch1,ach1> *v1, const Image<ch2,ach2> *v2)
{
    if (ch1 == ch2+ach2 && ach1 == 0) {
        // redo the colour
        Colour<ch2, ach2> v1_;
        for (chan_t c=0 ; c<ch1 ; ++c) v1_[c] = (*v1)[c];
        return image_zip_lua4<ch2, ach2, ch2, ach2, op, const Colour<ch2,ach2>*, const Image<ch2,ach2>*>(L, &v1_, v2);
    } else if (ch1!=ch2 && ach2==0 && ch1!=1 && ch2!=1 && ch1==ch2+1) {
        // redo the colour
        Colour<ch2, 1> v1_;
        for (chan_t c=0 ; c<ch1 ; ++c) v1_[c] = (*v1)[c];
        return image_zip_lua4<ch2, 1, ch2, ach2, op, const Colour<ch2,1>*, const Image<ch2,ach2>*>(L, &v1_, v2);
    } else {
        return image_zip_lua4<ch1, ach1, ch2, ach2, op, const Colour<ch1,ach1>*, const Image<ch2,ach2>*>(L, v1, v2);
    }
}

template<chan_t ch, chan_t ach, float op(float,float), class TA>
static ImageBase *image_zip_lua2 (lua_State *L, TA a)
{
    if (is_ptr(L, 2, IMAGE_TAG)) {
        const ImageBase *b = check_ptr<ImageBase>(L, 2, IMAGE_TAG);
        switch (b->channels()) {
            case 1:
            return image_zip_lua3<ch,ach,1,0,op>(L, a, static_cast<const Image<1,0>*>(b));
            case 2:
            if (b->hasAlpha()) {
                return image_zip_lua3<ch,ach,1,1,op>(L, a, static_cast<const Image<1,1>*>(b));
            } else {
                return image_zip_lua3<ch,ach,2,0,op>(L, a, static_cast<const Image<2,0>*>(b));
            }
            case 3:
            if (b->hasAlpha()) {
                return image_zip_lua3<ch,ach,2,1,op>(L, a, static_cast<const Image<2,1>*>(b));
            } else {
                return image_zip_lua3<ch,ach,3,0,op>(L, a, static_cast<const Image<3,0>*>(b));
            }
            case 4:
            if (b->hasAlpha()) {
                return image_zip_lua3<ch,ach,3,1,op>(L, a, static_cast<const Image<3,1>*>(b));
            } else {
                return image_zip_lua3<ch,ach,4,0,op>(L, a, static_cast<const Image<4,0>*>(b));
            }

            default: my_lua_error(L, "Internal error, strange number of channels.");
        }
        return NULL;
    }
    switch (get_colour_channels(L, 2)) {
        case 1: {
            Colour<1,0> colour;
            if (!check_colour(L,colour,2)) return NULL;
            return image_zip_lua3<ch,ach,1,0,op>(L, a, &colour);
        }

        case 2: {
            Colour<2,0> colour;
            if (!check_colour(L,colour,2)) return NULL;
            return image_zip_lua3<ch,ach,2,0,op>(L, a, &colour);
        }

        case 3: {
            Colour<3,0> colour;
            if (!check_colour(L,colour,2)) return NULL;
            return image_zip_lua3<ch,ach,3,0,op>(L, a, &colour);
        }

        case 4: {
            Colour<4,0> colour;
            if (!check_colour(L,colour,2)) return NULL;
            return image_zip_lua3<ch,ach,4,0,op>(L, a, &colour);
        }

        default:
        my_lua_error(L, "First argument should be an image or colour.");
    }
    return NULL;
}

template<float op(float,float)>
static ImageBase *image_zip_lua1 (lua_State *L)
{
    check_args(L,2);
    if (is_ptr(L, 1, IMAGE_TAG)) {
        const ImageBase *a = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        switch (a->channels()) {
            case 1:
            return image_zip_lua2<1,0,op>(L, static_cast<const Image<1,0>*>(a));
            case 2:
            if (a->hasAlpha()) {
                return image_zip_lua2<1,1,op>(L, static_cast<const Image<1,1>*>(a));
            } else {
                return image_zip_lua2<2,0,op>(L, static_cast<const Image<2,0>*>(a));
            }
            case 3:
            if (a->hasAlpha()) {
                return image_zip_lua2<2,1,op>(L, static_cast<const Image<2,1>*>(a));
            } else {
                return image_zip_lua2<3,0,op>(L, static_cast<const Image<3,0>*>(a));
            }
            case 4:
            if (a->hasAlpha()) {
                return image_zip_lua2<3,1,op>(L, static_cast<const Image<3,1>*>(a));
            } else {
                return image_zip_lua2<4,0,op>(L, static_cast<const Image<4,0>*>(a));
            }

            default: my_lua_error(L, "Internal error, strange number of channels.");
        }
        return NULL;
    }
    switch (get_colour_channels(L, 1)) {
        case 1: {
            Colour<1,0> colour;
            check_colour(L,colour,1);
            return image_zip_lua2<1,0,op>(L, &colour);
        }

        case 2: {
            Colour<2,0> colour;
            check_colour(L,colour,1);
            return image_zip_lua2<2,0,op>(L, &colour);
        }

        case 3: {
            Colour<3,0> colour;
            check_colour(L,colour,1);
            return image_zip_lua2<3,0,op>(L, &colour);
        }

        case 4: {
            Colour<4,0> colour;
            check_colour(L,colour,1);
            return image_zip_lua2<4,0,op>(L, &colour);
        }

        default:
        my_lua_error(L, "First argument should be an image or colour.");
    }
    return NULL;
}


// Our job here is to
// 1) call the right lerp function (regular, left_mask, right_mask)
// 2) figure out whether Colour<c,0> or Colour<c-1,1> was intended
// 3) raise an error if ch1/ach1/ch2/ach2 do not work together

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, class T1, class T2>
static void global_lerp_lua4 (lua_State *L, T1 v1, T2 v2, float param)
{
    // implements (1) and (3) above
    ImageBase *img = NULL;
    if (ch1==ch2 && ach1==ach2) {
        img = global_lerp_regular<ch1, ach1, ch2, ach2, T1, T2>(v1, v2, param);
    } else if (ch1 == 1 && ach1 == 0) {
        img = global_lerp_left_mask<ch2, ach2, T1, T2>(v1, v2, param);
    } else if (ch2 == 1 && ach2 == 0) {
        img = global_lerp_right_mask<ch1, ach1, T1, T2>(v1, v2, param);
    } else {
        my_lua_error(L, "Image lerp on incompatible images/colours.");
    }
    push_image(L, img);
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
static void global_lerp_lua3 (lua_State *L, const Image<ch1,ach1> *v1, const Image<ch2,ach2> *v2, float param)
{
    // both are images, must check size, but no issue with amibiguous vec(...)
    if (!v1->sizeCompatibleWith(v2)) {
        my_lua_error(L, "Operations require images have the same dimensions.");
    }
    global_lerp_lua4<ch1, ach1, ch2, ach2, const Image<ch1,ach1>*, const Image<ch2,ach2>*>(L, v1, v2, param);
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
static void global_lerp_lua3 (lua_State *L, const Colour<ch1,ach1> *v1, const Colour<ch2,ach2> *v2, float param)
{
    // implements (1) and (3) above
    if (ch1==ch2 && ach1==ach2) {
        Colour<ch2,ach2> colour = global_lerp_colour_regular<ch1, ach1, ch2, ach2>(v1, v2, param);
        push_colour(L, colour);
    } else if (ch1 == 1 && ach1 == 0) {
        Colour<ch2,ach2> colour = global_lerp_colour_left_mask<ch1, ach1, ch2, ach2>(v1, v2, param);
        push_colour(L, colour);
    } else if (ch2 == 1 && ach2 == 0) {
        Colour<ch1,ach1> colour = global_lerp_colour_right_mask<ch1, ach1, ch2, ach2>(v1, v2, param);
        push_colour(L, colour);
    } else {
        my_lua_error(L, "Image lerp on incompatible images/colours.");
    }
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
static void global_lerp_lua3 (lua_State *L, const Image<ch1,ach1> *v1, const Colour<ch2,ach2> *v2, float param)
{
    if (ch1+ach1 == ch2 && ach2 == 0) {
        // redo the colour
        Colour<ch1, ach1> v2_;
        for (chan_t c=0 ; c<ch2 ; ++c) v2_[c] = (*v2)[c];
        global_lerp_lua4<ch1, ach1, ch1, ach1, const Image<ch1,ach1>*, const Colour<ch1,ach1>*>(L, v1, &v2_, param);
    } else {
        global_lerp_lua4<ch1, ach1, ch2, ach2, const Image<ch1,ach1>*, const Colour<ch2,ach2>*>(L, v1, v2, param);
    }
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
static void global_lerp_lua3 (lua_State *L, const Colour<ch1,ach1> *v1, const Image<ch2,ach2> *v2, float param)
{
    if (ch1 == ch2+ach2 && ach1 == 0) {
        // redo the colour
        Colour<ch2, ach2> v1_;
        for (chan_t c=0 ; c<ch1 ; ++c) v1_[c] = (*v1)[c];
        global_lerp_lua4<ch2, ach2, ch2, ach2, const Colour<ch2,ach2>*, const Image<ch2,ach2>*>(L, &v1_, v2, param);
    } else {
        global_lerp_lua4<ch1, ach1, ch2, ach2, const Colour<ch1,ach1>*, const Image<ch2,ach2>*>(L, v1, v2, param);
    }
}

template<chan_t ch, chan_t ach, class TA>
static void global_lerp_lua2 (lua_State *L, TA a)
{
    float param = luaL_checknumber(L, 3);
    if (is_ptr(L, 2, IMAGE_TAG)) {
        const ImageBase *b = check_ptr<ImageBase>(L, 2, IMAGE_TAG);
        switch (b->channels()) {
            case 1:
            global_lerp_lua3<ch,ach,1,0>(L, a, static_cast<const Image<1,0>*>(b), param);
            break;
            case 2:
            if (b->hasAlpha()) {
                global_lerp_lua3<ch,ach,1,1>(L, a, static_cast<const Image<1,1>*>(b), param);
            } else {
                global_lerp_lua3<ch,ach,2,0>(L, a, static_cast<const Image<2,0>*>(b), param);
            } break;
            case 3:
            if (b->hasAlpha()) {
                global_lerp_lua3<ch,ach,2,1>(L, a, static_cast<const Image<2,1>*>(b), param);
            } else {
                global_lerp_lua3<ch,ach,3,0>(L, a, static_cast<const Image<3,0>*>(b), param);
            } break;
            case 4:
            if (b->hasAlpha()) {
                global_lerp_lua3<ch,ach,3,1>(L, a, static_cast<const Image<3,1>*>(b), param);
            } else {
                global_lerp_lua3<ch,ach,4,0>(L, a, static_cast<const Image<4,0>*>(b), param);
            } break;

            default: my_lua_error(L, "Internal error, strange number of channels.");
        }
        return;
    }
    switch (get_colour_channels(L, 2)) {
        case 1: {
            Colour<1,0> colour;
            if (!check_colour(L,colour,2)) return;
            global_lerp_lua3<ch,ach,1,0>(L, a, &colour, param);
            break;
        }

        case 2: {
            Colour<2,0> colour;
            if (!check_colour(L,colour,2)) return;
            global_lerp_lua3<ch,ach,2,0>(L, a, &colour, param);
            break;
        }

        case 3: {
            Colour<3,0> colour;
            if (!check_colour(L,colour,2)) return;
            global_lerp_lua3<ch,ach,3,0>(L, a, &colour, param);
            break;
        }

        case 4: {
            Colour<4,0> colour;
            if (!check_colour(L,colour,2)) return;
            global_lerp_lua3<ch,ach,4,0>(L, a, &colour, param);
            break;
        }

        default:
        my_lua_error(L, "First argument should be an image or colour.");
    }
}

static void global_lerp_lua1 (lua_State *L)
{
    check_args(L,3);
    if (is_ptr(L, 1, IMAGE_TAG)) {
        const ImageBase *a = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        switch (a->channels()) {
            case 1:
            global_lerp_lua2<1,0>(L, static_cast<const Image<1,0>*>(a));
            break;
            case 2:
            if (a->hasAlpha()) {
                global_lerp_lua2<1,1>(L, static_cast<const Image<1,1>*>(a));
            } else {
                global_lerp_lua2<2,0>(L, static_cast<const Image<2,0>*>(a));
            } break;
            case 3:
            if (a->hasAlpha()) {
                global_lerp_lua2<2,1>(L, static_cast<const Image<2,1>*>(a));
            } else {
                global_lerp_lua2<3,0>(L, static_cast<const Image<3,0>*>(a));
            } break;
            case 4:
            if (a->hasAlpha()) {
                global_lerp_lua2<3,1>(L, static_cast<const Image<3,1>*>(a));
            } else {
                global_lerp_lua2<4,0>(L, static_cast<const Image<4,0>*>(a));
            } break;

            default: my_lua_error(L, "Internal error, strange number of channels.");
        }
        return;
    }
    switch (get_colour_channels(L, 1)) {
        case 1: {
            Colour<1,0> colour;
            check_colour(L,colour,1);
            global_lerp_lua2<1,0>(L, &colour);
            break;
        }

        case 2: {
            Colour<2,0> colour;
            check_colour(L,colour,1);
            global_lerp_lua2<2,0>(L, &colour);
            break;
        }

        case 3: {
            Colour<3,0> colour;
            check_colour(L,colour,1);
            global_lerp_lua2<3,0>(L, &colour);
            break;
        }

        case 4: {
            Colour<4,0> colour;
            check_colour(L,colour,1);
            global_lerp_lua2<4,0>(L, &colour);
            break;
        }

        default:
        my_lua_error(L, "First argument should be an image or colour.");
    }
}


template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float zop(float,float), float rop(float, float), class T1, class T2>
static ColourBase *image_zip_reduce_lua4 (lua_State *L, T1 v1, T2 v2)
{
    // implements (1) and (3) above
    if (ch1 == ch2 && ach1 == ach2)
        return image_zip_reduce_regular<ch1, ach1, ch2, ach2, zop, rop, T1, T2>(v1, v2);
    if (ch1 == 1 && ach1 == 0)
        return image_zip_reduce_left_mask<ch2, ach2, zop, rop, T1, T2>(v1, v2);
    if (ch2 == 1 && ach2 == 0)
        return image_zip_reduce_right_mask<ch1, ach1, zop, rop, T1, T2>(v1, v2);
    my_lua_error(L, "Image operation on incompatible images/colours.");
    return NULL;
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float zop(float,float), float rop(float, float)>
static ColourBase *image_zip_reduce_lua3 (lua_State *L, const Image<ch1,ach1> *v1, const Image<ch2,ach2> *v2)
{
    // both are images, must check size, but no issue with amibiguous vec(...)
    if (!v1->sizeCompatibleWith(v2)) {
        my_lua_error(L, "Operations require images have the same dimensions.");
    }
    return image_zip_reduce_lua4<ch1, ach1, ch2, ach2, zop, rop, const Image<ch1,ach1>*, const Image<ch2,ach2>*>(L, v1, v2);
    
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float zop(float,float), float rop(float, float)>
static ColourBase *image_zip_reduce_lua3 (lua_State *, const Colour<ch1,ach1> *, const Colour<ch2,ach2> *)
{
    abort();
    return NULL;
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float zop(float,float), float rop(float, float)>
static ColourBase *image_zip_reduce_lua3 (lua_State *L, const Image<ch1,ach1> *v1, const Colour<ch2,ach2> *v2)
{
    if (ch1+ach1 == ch2) {
        // redo the colour
        Colour<ch1, ach1> v2_;
        for (chan_t c=0 ; c<ch2 ; ++c) v2_[c] = (*v2)[c];
        return image_zip_reduce_lua4<ch1, ach1, ch1, ach1, zop, rop, const Image<ch1,ach1>*, const Colour<ch1,ach1>*>(L, v1, &v2_);
    } else {
        return image_zip_reduce_lua4<ch1, ach1, ch2, ach2, zop, rop, const Image<ch1,ach1>*, const Colour<ch2,ach2>*>(L, v1, v2);
    }
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, float zop(float,float), float rop(float, float)>
static ColourBase *image_zip_reduce_lua3 (lua_State *L, const Colour<ch1,ach1> *v1, const Image<ch2,ach2> *v2)
{
    if (ch1 == ch2+ach2) {
        // redo the colour
        Colour<ch2, ach2> v1_;
        for (chan_t c=0 ; c<ch1 ; ++c) v1_[c] = (*v1)[c];
        return image_zip_reduce_lua4<ch2, ach2, ch2, ach2, zop, rop, const Colour<ch2,ach2>*, const Image<ch2,ach2>*>(L, &v1_, v2);
    } else {
        return image_zip_reduce_lua4<ch1, ach1, ch2, ach2, zop, rop, const Colour<ch1,ach1>*, const Image<ch2,ach2>*>(L, v1, v2);
    }
}

template<chan_t ch, chan_t ach, float zop(float,float), float rop(float, float), class TA>
static ColourBase *image_zip_reduce_lua2 (lua_State *L, TA a, const ImageBase *&some_image)
{
    if (is_ptr(L, 2, IMAGE_TAG)) {
        const ImageBase *b = check_ptr<ImageBase>(L, 2, IMAGE_TAG);
        some_image = b;
        switch (b->channels()) {
            case 1:
            return image_zip_reduce_lua3<ch,ach,1,0,zop,rop>(L, a, static_cast<const Image<1,0>*>(b));
            case 2:
            if (b->hasAlpha()) {
                return image_zip_reduce_lua3<ch,ach,1,1,zop,rop>(L, a, static_cast<const Image<1,1>*>(b));
            } else {
                return image_zip_reduce_lua3<ch,ach,2,0,zop,rop>(L, a, static_cast<const Image<2,0>*>(b));
            }
            case 3:
            if (b->hasAlpha()) {
                return image_zip_reduce_lua3<ch,ach,2,1,zop,rop>(L, a, static_cast<const Image<2,1>*>(b));
            } else {
                return image_zip_reduce_lua3<ch,ach,3,0,zop,rop>(L, a, static_cast<const Image<3,0>*>(b));
            }
            case 4:
            if (b->hasAlpha()) {
                return image_zip_reduce_lua3<ch,ach,3,1,zop,rop>(L, a, static_cast<const Image<3,1>*>(b));
            } else {
                return image_zip_reduce_lua3<ch,ach,4,0,zop,rop>(L, a, static_cast<const Image<4,0>*>(b));
            }

            default: my_lua_error(L, "Internal error, strange number of channels.");
        }
        return NULL;
    }
    switch (get_colour_channels(L, 2)) {
        case 1: {
            Colour<1,0> colour;
            if (!check_colour(L,colour,2)) return NULL;
            return image_zip_reduce_lua3<ch,ach,1,0,zop,rop>(L, a, &colour);
        }

        case 2: {
            Colour<2,0> colour;
            if (!check_colour(L,colour,2)) return NULL;
            return image_zip_reduce_lua3<ch,ach,2,0,zop,rop>(L, a, &colour);
        }

        case 3: {
            Colour<3,0> colour;
            if (!check_colour(L,colour,2)) return NULL;
            return image_zip_reduce_lua3<ch,ach,3,0,zop,rop>(L, a, &colour);
        }

        case 4: {
            Colour<4,0> colour;
            if (!check_colour(L,colour,2)) return NULL;
            return image_zip_reduce_lua3<ch,ach,4,0,zop,rop>(L, a, &colour);
        }

        default:
        my_lua_error(L, "First argument should be an image or colour.");
    }
    return NULL;
}

template<float zop(float,float), float rop(float, float)>
static ColourBase *image_zip_reduce_lua1 (lua_State *L, const ImageBase *&some_image)
{
    check_args(L,2);
    if (is_ptr(L, 1, IMAGE_TAG)) {
        const ImageBase *a = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        some_image = a;
        switch (a->channels()) {
            case 1:
            return image_zip_reduce_lua2<1,0,zop,rop>(L, static_cast<const Image<1,0>*>(a), some_image);
            case 2:
            if (a->hasAlpha()) {
                return image_zip_reduce_lua2<1,1,zop,rop>(L, static_cast<const Image<1,1>*>(a), some_image);
            } else {
                return image_zip_reduce_lua2<2,0,zop,rop>(L, static_cast<const Image<2,0>*>(a), some_image);
            }
            case 3:
            if (a->hasAlpha()) {
                return image_zip_reduce_lua2<2,1,zop,rop>(L, static_cast<const Image<2,1>*>(a), some_image);
            } else {
                return image_zip_reduce_lua2<3,0,zop,rop>(L, static_cast<const Image<3,0>*>(a), some_image);
            }
            case 4:
            if (a->hasAlpha()) {
                return image_zip_reduce_lua2<3,1,zop,rop>(L, static_cast<const Image<3,1>*>(a), some_image);
            } else {
                return image_zip_reduce_lua2<4,0,zop,rop>(L, static_cast<const Image<4,0>*>(a), some_image);
            }

            default: my_lua_error(L, "Internal error, strange number of channels.");
        }
        return NULL;
    }
    switch (get_colour_channels(L, 1)) {
        case 1: {
            Colour<1,0> colour;
            check_colour(L,colour,1);
            return image_zip_reduce_lua2<1,0,zop,rop>(L, &colour, some_image);
        }

        case 2: {
            Colour<2,0> colour;
            check_colour(L,colour,1);
            return image_zip_reduce_lua2<2,0,zop,rop>(L, &colour, some_image);
        }

        case 3: {
            Colour<3,0> colour;
            check_colour(L,colour,1);
            return image_zip_reduce_lua2<3,0,zop,rop>(L, &colour, some_image);
        }

        case 4: {
            Colour<4,0> colour;
            check_colour(L,colour,1);
            return image_zip_reduce_lua2<4,0,zop,rop>(L, &colour, some_image);
        }

        default:
        my_lua_error(L, "First argument should be an image or colour.");
    }
    return NULL;
}

// Our job here is to
// 1) call the right image_op function (regular, left_mask, right_mask)
// 2) figure out whether Colour<c,0> or Colour<c-1,1> was intended
// 3) raise an error if ch1/ach1/ch2/ach2 do not work together

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2, class T1, class T2>
static ImageBase *image_blend_lua4 (lua_State *L, T1 v1, T2 v2)
{
    // implements (1) and (3) above
    if (ch1 == ch2)
        return image_blend_regular<ch1, ach1, ch2, ach2, T1, T2>(v1, v2);
    if (ch1 == 1 && ach1 == 0)
        return image_blend_left_mask<ch2, ach2, T1, T2>(v1, v2);
    if (ch2 == 1 && ach2 == 0)
        return image_blend_right_mask<ch1, ach1, T1, T2>(v1, v2);
    my_lua_error(L, "Image blend of incompatible images/colours.");
    return NULL;
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
static ImageBase *image_blend_lua3 (lua_State *L, const Image<ch1,ach1> *v1, const Image<ch2,ach2> *v2)
{
    // both are images, must check size, but no issue with amibiguous vec(...)
    if (!v1->sizeCompatibleWith(v2)) {
        my_lua_error(L, "Operations require images have the same dimensions.");
    }
    return image_blend_lua4<ch1, ach1, ch2, ach2, const Image<ch1,ach1>*, const Image<ch2,ach2>*>(L, v1, v2);
    
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
static ImageBase *image_blend_lua3 (lua_State *, const Colour<ch1,ach1> *, const Colour<ch2,ach2> *)
{
    abort();
    return NULL;
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
static ImageBase *image_blend_lua3 (lua_State *L, const Image<ch1,ach1> *v1, const Colour<ch2,ach2> *v2)
{
    if (ch1+ach1==ch2 && ach1==1 && ach2==0) {
        // redo the colour
        Colour<ch1, ach1> v2_;
        for (chan_t c=0 ; c<ch2 ; ++c) v2_[c] = (*v2)[c];
        return image_blend_lua4<ch1, ach1, ch1, ach1, const Image<ch1,ach1>*, const Colour<ch1,ach1>*>(L, v1, &v2_);
    } else {
        return image_blend_lua4<ch1, ach1, ch2, ach2, const Image<ch1,ach1>*, const Colour<ch2,ach2>*>(L, v1, v2);
    }
}

template<chan_t ch1, chan_t ach1, chan_t ch2, chan_t ach2>
static ImageBase *image_blend_lua3 (lua_State *L, const Colour<ch1,ach1> *v1, const Image<ch2,ach2> *v2)
{
    if (ach1 != 0) abort();
    if (ch1==ch2+1 && (ach2==1 || (ach2==0 && ch2>1))) {
        // e.g. vec(1,1,1,1)..make(sz, 3, true)
        // if ach2==0 then we have to worry about a mask image on the right, so require ch2>1 
        // e.g. vec(1,1,1,1)..make(sz, 3, false)
        // redo the colour, it was assumed to have no alpha channel but only fits if it does have alpha channel 
        Colour<ch2, 1> v1_;
        for (chan_t c=0 ; c<ch1 ; ++c) v1_[c] = (*v1)[c];
        return image_blend_lua4<ch2, 1, ch2, ach2, const Colour<ch2,1>*, const Image<ch2,ach2>*>(L, &v1_, v2);
    } else {
        return image_blend_lua4<ch1, ach1, ch2, ach2, const Colour<ch1,ach1>*, const Image<ch2,ach2>*>(L, v1, v2);
    }
}

template<chan_t ch, chan_t ach, class TA>
static ImageBase *image_blend_lua2 (lua_State *L, TA a)
{
    if (is_ptr(L, 2, IMAGE_TAG)) {
        const ImageBase *b = check_ptr<ImageBase>(L, 2, IMAGE_TAG);
        switch (b->channels()) {
            case 1:
            return image_blend_lua3<ch,ach,1,0>(L, a, static_cast<const Image<1,0>*>(b));
            case 2:
            if (b->hasAlpha()) {
                return image_blend_lua3<ch,ach,1,1>(L, a, static_cast<const Image<1,1>*>(b));
            } else {
                return image_blend_lua3<ch,ach,2,0>(L, a, static_cast<const Image<2,0>*>(b));
            }
            case 3:
            if (b->hasAlpha()) {
                return image_blend_lua3<ch,ach,2,1>(L, a, static_cast<const Image<2,1>*>(b));
            } else {
                return image_blend_lua3<ch,ach,3,0>(L, a, static_cast<const Image<3,0>*>(b));
            }
            case 4:
            if (b->hasAlpha()) {
                return image_blend_lua3<ch,ach,3,1>(L, a, static_cast<const Image<3,1>*>(b));
            } else {
                return image_blend_lua3<ch,ach,4,0>(L, a, static_cast<const Image<4,0>*>(b));
            }

            default: my_lua_error(L, "Internal error, strange number of channels.");
        }
        return NULL;
    }
    switch (get_colour_channels(L, 2)) {
        case 1: {
            Colour<1,0> colour;
            if (!check_colour(L,colour,2)) return NULL;
            return image_blend_lua3<ch,ach,1,0>(L, a, &colour);
        }

        case 2: {
            Colour<2,0> colour;
            if (!check_colour(L,colour,2)) return NULL;
            return image_blend_lua3<ch,ach,2,0>(L, a, &colour);
        }

        case 3: {
            Colour<3,0> colour;
            if (!check_colour(L,colour,2)) return NULL;
            return image_blend_lua3<ch,ach,3,0>(L, a, &colour);
        }

        case 4: {
            Colour<4,0> colour;
            if (!check_colour(L,colour,2)) return NULL;
            return image_blend_lua3<ch,ach,4,0>(L, a, &colour);
        }

        default:
        my_lua_error(L, "First argument should be an image or colour.");
    }
    return NULL;
}

static ImageBase *image_blend_lua1 (lua_State *L)
{
    check_args(L,2);
    if (is_ptr(L, 1, IMAGE_TAG)) {
        const ImageBase *a = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        switch (a->channels()) {
            case 1:
            return image_blend_lua2<1,0>(L, static_cast<const Image<1,0>*>(a));
            case 2:
            if (a->hasAlpha()) {
                return image_blend_lua2<1,1>(L, static_cast<const Image<1,1>*>(a));
            } else {
                return image_blend_lua2<2,0>(L, static_cast<const Image<2,0>*>(a));
            }
            case 3:
            if (a->hasAlpha()) {
                return image_blend_lua2<2,1>(L, static_cast<const Image<2,1>*>(a));
            } else {
                return image_blend_lua2<3,0>(L, static_cast<const Image<3,0>*>(a));
            }
            case 4:
            if (a->hasAlpha()) {
                return image_blend_lua2<3,1>(L, static_cast<const Image<3,1>*>(a));
            } else {
                return image_blend_lua2<4,0>(L, static_cast<const Image<4,0>*>(a));
            }

            default: my_lua_error(L, "Internal error, strange number of channels.");
        }
        return NULL;
    }
    switch (get_colour_channels(L, 1)) {
        case 1: {
            Colour<1,0> colour;
            if (!check_colour(L,colour,1)) return NULL;
            return image_blend_lua2<1,0>(L, &colour);
        }

        case 2: {
            Colour<2,0> colour;
            if (!check_colour(L,colour,1)) return NULL;
            return image_blend_lua2<2,0>(L, &colour);
        }

        case 3: {
            Colour<3,0> colour;
            if (!check_colour(L,colour,1)) return NULL;
            return image_blend_lua2<3,0>(L, &colour);
        }

        case 4: {
            Colour<4,0> colour;
            if (!check_colour(L,colour,1)) return NULL;
            return image_blend_lua2<4,0>(L, &colour);
        }

        default:
        my_lua_error(L, "First argument should be an image or colour.");
    }
    return NULL;
}


static int image_gc (lua_State *L)
{ 
    check_args(L, 1); 
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    lua_extmemburden(L, -(long)self->numBytes());
    delete self; 
    return 0; 
}

static int image_eq (lua_State *L)
{
    check_args(L, 2); 
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    ImageBase *that = check_ptr<ImageBase>(L, 2, IMAGE_TAG);
    lua_pushboolean(L, self==that); 
    return 1; 
}

static int image_tostring (lua_State *L)
{
    check_args(L,1);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    std::stringstream ss;
    ss << self;
    push_string(L, ss.str());
    return 1;
}

static int image_save (lua_State *L)
{
HANDLE_BEGIN
    check_args(L,2);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    std::string filename = lua_tostring(L, 2);
    image_save(self, filename);
    return 0;
HANDLE_END
}

template<chan_t ch, chan_t ach> void foreach (lua_State *L, const ImageBase *self_, int func_index)
{
    const Image<ch,ach> *self = static_cast<const Image<ch,ach>*>(self_);
    for (uimglen_t y=0 ; y<self->height ; ++y) {
        for (uimglen_t x=0 ; x<self->width ; ++x) {
            lua_pushvalue(L, func_index);
            push_colour(L, self->pixel(x,y));
            lua_pushvector2(L, x, y);
            int status = lua_pcall(L, 2, 0, 0); 
            if (status != 0) {
                const char *msg = lua_tostring(L, -1);
                std::stringstream ss;
                ss << "During foreach on image at (" << x << "," << y << "): " << msg;
                my_lua_error(L, ss.str());
            }
        }
    }   
}

static int image_foreach (lua_State *L)
{
    check_args(L,2);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    check_is_function(L, 2);
    int fi = 2;

    if (self->hasAlpha()) {
        switch (self->channels()) {
            case 2: foreach<1,1>(L, self, fi); break;
            case 3: foreach<2,1>(L, self, fi); break;
            case 4: foreach<3,1>(L, self, fi); break;
            default:
            my_lua_error(L, "Channels must be either 2, 3, or 4.");
        }
    } else {
        switch (self->channels()) {
            case 1: foreach<1,0>(L, self, fi); break;
            case 2: foreach<2,0>(L, self, fi); break;
            case 3: foreach<3,0>(L, self, fi); break;
            case 4: foreach<4,0>(L, self, fi); break;
            default:
            my_lua_error(L, "Channels must be either 1, 2, 3, or 4.");
        }
    }

    return 0;
}

template<chan_t src_ch, chan_t src_ach, chan_t dst_ch, chan_t dst_ach>
ImageBase *map_with_lua_func (lua_State *L, const ImageBase *src_, int func_index)
{
    const Image<src_ch, src_ach> *src = static_cast<const Image<src_ch, src_ach>*>(src_);
    uimglen_t width = src->width;
    uimglen_t height = src->height;
    Image<dst_ch, dst_ach> *dst = new Image<dst_ch, dst_ach>(width, height);
    Colour<dst_ch, dst_ach> p(0);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            lua_pushvalue(L, func_index);
            push_colour(L, src->pixel(x,y));
            lua_pushvector2(L, x, y);
            int status = lua_pcall(L, 2, 1, 0); 
            if (status == 0) {
                if (!check_colour(L, p, -1)) {
                    delete dst;
                    const char *msg = lua_tostring(L, -1);
                    std::stringstream ss;
                    ss << "While mapping the image at (" << x << "," << y << "): returned value \""<<msg<<"\" has the wrong type.";
                    my_lua_error(L, ss.str());
                }
                dst->pixel(x,y) = p;
            } else {
                const char *msg = lua_tostring(L, -1);
                delete dst;
                std::stringstream ss;
                ss << "While mapping the image at (" << x << "," << y << "): " << msg;
                my_lua_error(L, ss.str());
            }
            lua_pop(L, 1);
        }   
    }   
    return dst;
}

static int image_map (lua_State *L)
{
    ImageBase *src;
    chan_t dst_ch;
    bool dst_ach = false;
    int fi;
    if (lua_gettop(L) == 4) {
        src = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        dst_ch = check_int(L, 2, 1, 4);
        dst_ach = check_bool(L, 3);
        check_is_function(L, 4);
        if (dst_ach && dst_ch==4) my_lua_error(L, "Image with alpha channel can have at most 3 colour channels.");
        fi = 4;
    } else {
        check_args(L,3);
        src = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        dst_ch = check_int(L, 2, 1, 4);
        check_is_function(L, 3);
        fi = 3;
    }

    if (dst_ach) dst_ch++;

    chan_t src_ch = src->channels();
    ImageBase *out = NULL;

    switch (src_ch) {
        case 1:
        switch (dst_ch) {
            case 1: out =                                                    map_with_lua_func<1,0,1,0>(L, src, fi); break;
            case 2: out = dst_ach ? map_with_lua_func<1,0,1,1>(L, src, fi) : map_with_lua_func<1,0,2,0>(L, src, fi); break;
            case 3: out = dst_ach ? map_with_lua_func<1,0,2,1>(L, src, fi) : map_with_lua_func<1,0,3,0>(L, src, fi); break;
            case 4: out = dst_ach ? map_with_lua_func<1,0,3,1>(L, src, fi) : map_with_lua_func<1,0,4,0>(L, src, fi); break;
            default:
            my_lua_error(L, "Dest channels must be either 1, 2, 3, or 4.");
        }
        break;

        case 2:
        if (src->hasAlpha()) {
            switch (dst_ch) {
                case 1: out =                                                    map_with_lua_func<1,1,1,0>(L, src, fi); break;
                case 2: out = dst_ach ? map_with_lua_func<1,1,1,1>(L, src, fi) : map_with_lua_func<1,1,2,0>(L, src, fi); break;
                case 3: out = dst_ach ? map_with_lua_func<1,1,2,1>(L, src, fi) : map_with_lua_func<1,1,3,0>(L, src, fi); break;
                case 4: out = dst_ach ? map_with_lua_func<1,1,3,1>(L, src, fi) : map_with_lua_func<1,1,4,0>(L, src, fi); break;
                default:
                my_lua_error(L, "Dest channels must be either 1, 2, 3, or 4.");
            }
        } else {
            switch (dst_ch) {
                case 1: out =                                                    map_with_lua_func<2,0,1,0>(L, src, fi); break;
                case 2: out = dst_ach ? map_with_lua_func<2,0,1,1>(L, src, fi) : map_with_lua_func<2,0,2,0>(L, src, fi); break;
                case 3: out = dst_ach ? map_with_lua_func<2,0,2,1>(L, src, fi) : map_with_lua_func<2,0,3,0>(L, src, fi); break;
                case 4: out = dst_ach ? map_with_lua_func<2,0,3,1>(L, src, fi) : map_with_lua_func<2,0,4,0>(L, src, fi); break;
                default:
                my_lua_error(L, "Dest channels must be either 1, 2, 3, or 4.");
            }
        }
        break;

        case 3:
        if (src->hasAlpha()) {
            switch (dst_ch) {
                case 1: out =                                                    map_with_lua_func<2,1,1,0>(L, src, fi); break;
                case 2: out = dst_ach ? map_with_lua_func<2,1,1,1>(L, src, fi) : map_with_lua_func<2,1,2,0>(L, src, fi); break;
                case 3: out = dst_ach ? map_with_lua_func<2,1,2,1>(L, src, fi) : map_with_lua_func<2,1,3,0>(L, src, fi); break;
                case 4: out = dst_ach ? map_with_lua_func<2,1,3,1>(L, src, fi) : map_with_lua_func<2,1,4,0>(L, src, fi); break;
                default:
                my_lua_error(L, "Dest channels must be either 1, 2, 3, or 4.");
            }
        } else {
            switch (dst_ch) {
                case 1: out =                                                    map_with_lua_func<3,0,1,0>(L, src, fi); break;
                case 2: out = dst_ach ? map_with_lua_func<3,0,1,1>(L, src, fi) : map_with_lua_func<3,0,2,0>(L, src, fi); break;
                case 3: out = dst_ach ? map_with_lua_func<3,0,2,1>(L, src, fi) : map_with_lua_func<3,0,3,0>(L, src, fi); break;
                case 4: out = dst_ach ? map_with_lua_func<3,0,3,1>(L, src, fi) : map_with_lua_func<3,0,4,0>(L, src, fi); break;
                default:
                my_lua_error(L, "Dest channels must be either 1, 2, 3, or 4.");
            }
        }
        break;

        case 4:
        if (src->hasAlpha()) {
            switch (dst_ch) {
                case 1: out =                                                    map_with_lua_func<3,1,1,0>(L, src, fi); break;
                case 2: out = dst_ach ? map_with_lua_func<3,1,1,1>(L, src, fi) : map_with_lua_func<3,1,2,0>(L, src, fi); break;
                case 3: out = dst_ach ? map_with_lua_func<3,1,2,1>(L, src, fi) : map_with_lua_func<3,1,3,0>(L, src, fi); break;
                case 4: out = dst_ach ? map_with_lua_func<3,1,3,1>(L, src, fi) : map_with_lua_func<3,1,4,0>(L, src, fi); break;
                default:
                my_lua_error(L, "Dest channels must be either 1, 2, 3, or 4.");
            }
        } else {
            switch (dst_ch) {
                case 1: out =                                                    map_with_lua_func<4,0,1,0>(L, src, fi); break;
                case 2: out = dst_ach ? map_with_lua_func<4,0,1,1>(L, src, fi) : map_with_lua_func<4,0,2,0>(L, src, fi); break;
                case 3: out = dst_ach ? map_with_lua_func<4,0,2,1>(L, src, fi) : map_with_lua_func<4,0,3,0>(L, src, fi); break;
                case 4: out = dst_ach ? map_with_lua_func<4,0,3,1>(L, src, fi) : map_with_lua_func<4,0,4,0>(L, src, fi); break;
                default:
                my_lua_error(L, "Dest channels must be either 1, 2, 3, or 4.");
            }
        }
        break;

        default:
        my_lua_error(L, "Source channels must be either 1, 2, 3, or 4.");
    }

    push_image(L, out);
    return 1;
}

template<chan_t ch, chan_t ach>
void reduce_with_lua_func (lua_State *L, const ImageBase *self_, Colour<ch,ach> zero, int func_index)
{
    const Image<ch,ach> *self = static_cast<const Image<ch,ach>*>(self_);

    for (uimglen_t y=0 ; y<self->height ; ++y) {
        for (uimglen_t x=0 ; x<self->width ; ++x) {
            lua_pushvalue(L, func_index);
            push_colour(L, zero);
            push_colour(L, self->pixel(x,y));
            lua_pushvector2(L, x, y);
            int status = lua_pcall(L, 3, 1, 0); 
            if (status == 0) {
                if (!check_colour(L, zero, -1)) {
                    const char *msg = lua_tostring(L, -1);
                    std::stringstream ss;
                    ss << "While reducing the image at (" << x << "," << y << "): returned value \""<<msg<<"\" has the wrong type.";
                    my_lua_error(L, ss.str());
                }
            } else {
                const char *msg = lua_tostring(L, -1);
                std::stringstream ss;
                ss << "While mapping the image at (" << x << "," << y << "): " << msg;
                my_lua_error(L, ss.str());
            }
            lua_pop(L, 1);
        }   
    }
    push_colour(L, zero);
}

static int image_reduce (lua_State *L)
{
    check_args(L,3);
    // img:A, zero:A, func:A,A -> A
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    int pi = 2;
    check_is_function(L, 3);
    int fi = 3;
    switch (self->channels()) {
        case 1:
        if (self->hasAlpha()) {
            my_lua_error(L, "Internal error");
        } else {
            Colour<1,0> p;
            if (!check_colour(L, p, pi)) my_lua_error(L, "Reduce 'zero' value had the wrong number of elements.");
            else reduce_with_lua_func(L, self, p, fi);
        }
        break;

        case 2:
        if (self->hasAlpha()) {
            Colour<1,1> p;
            if (!check_colour(L, p, pi)) my_lua_error(L, "Reduce 'zero' value had the wrong number of elements.");
            else reduce_with_lua_func(L, self, p, fi);
        } else {
            Colour<2,0> p;
            if (!check_colour(L, p, pi)) my_lua_error(L, "Reduce 'zero' value had the wrong number of elements.");
            else reduce_with_lua_func(L, self, p, fi);
        }
        break;

        case 3:
        if (self->hasAlpha()) {
            Colour<2,1> p;
            if (!check_colour(L, p, pi)) my_lua_error(L, "Reduce 'zero' value had the wrong number of elements.");
            else reduce_with_lua_func(L, self, p, fi);
        } else {
            Colour<3,0> p;
            if (!check_colour(L, p, pi)) my_lua_error(L, "Reduce 'zero' value had the wrong number of elements.");
            else reduce_with_lua_func(L, self, p, fi);
        }
        break;

        case 4:
        if (self->hasAlpha()) {
            Colour<3,1> p;
            if (!check_colour(L, p, pi)) my_lua_error(L, "Reduce 'zero' value had the wrong number of elements.");
            else reduce_with_lua_func(L, self, p, fi);
        } else {
            Colour<4,0> p;
            if (!check_colour(L, p, pi)) my_lua_error(L, "Reduce 'zero' value had the wrong number of elements.");
            else reduce_with_lua_func(L, self, p, fi);
        }
        break;

        default:
        my_lua_error(L, "Image must have either 1, 2, 3, or 4 channels.");
    }
    return 1;
}

static int image_crop (lua_State *L)
{
    if (lua_gettop(L) == 3) {
        ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        simglen_t left, bottom;
        check_scoord(L, 2, left, bottom);
        uimglen_t width, height;
        check_coord(L, 3, width, height);
        push_image(L, self->crop(left,bottom,width,height,NULL));
    } else {
        check_args(L,4);
        ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        simglen_t left, bottom;
        check_scoord(L, 2, left, bottom);
        uimglen_t width, height;
        check_coord(L, 3, width, height);
        ColourBase *colour = alloc_colour(L, self->channels(), self->hasAlpha(), 4);
        push_image(L, self->crop(left,bottom,width,height,colour));
        delete colour;
    }
    return 1;
}

static int image_crop_centre (lua_State *L)
{
    if (lua_gettop(L) == 2) {
        ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        uimglen_t width, height;
        check_coord(L, 2, width, height);
        simglen_t left = (simglen_t(self->width) - simglen_t(width))/2;
        simglen_t bottom = (simglen_t(self->height) - simglen_t(height))/2;
        push_image(L, self->crop(left,bottom,width,height,NULL));
    } else {
        check_args(L,3);
        ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        uimglen_t width, height;
        check_coord(L, 2, width, height);
        simglen_t left = (simglen_t(self->width) - simglen_t(width))/2;
        simglen_t bottom = (simglen_t(self->height) - simglen_t(height))/2;
        ColourBase *colour = alloc_colour(L, self->channels(), self->hasAlpha(), 3);
        push_image(L, self->crop(left,bottom,width,height,colour));
        delete colour;
    }
    return 1;
}

ScaleFilter scale_filter_from_string (const std::string &s)
{
    if (s == "BOX") return SF_BOX;
    if (s == "BILINEAR") return SF_BILINEAR;
    if (s == "BSPLINE") return SF_BSPLINE;
    if (s == "BICUBIC") return SF_BICUBIC;
    if (s == "CATMULLROM") return SF_CATMULLROM;
    if (s == "LANCZOS3") return SF_LANCZOS3;
    EXCEPT << "Expected BOX, BILINEAR, BSPLINE, BICUBIC, CATMULLROM, or LANCZOS3.  Got: \"" << s << "\"" << ENDL;
}

static int image_scale (lua_State *L)
{
HANDLE_BEGIN
    check_args(L, 3);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    uimglen_t width, height;
    check_coord(L, 2, width, height);
    std::string filter_type = luaL_checkstring(L, 3);
    ImageBase *out = self->scale(width, height, scale_filter_from_string(filter_type));
    push_image(L, out);
    return 1;
HANDLE_END
}

static int image_scale_by (lua_State *L)
{
HANDLE_BEGIN
    check_args(L, 3);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    float x_=0, y_=0;
    switch (lua_type(L,2)) {
        case LUA_TNUMBER:
        x_ = y_ = lua_tonumber(L,2);
        break;
        case LUA_TVECTOR2:
        lua_checkvector2(L, 2, &x_, &y_);
        break;
        default:
        my_lua_error(L, "Scale must be a number or vector2 (got "+type_name(L,2)+")");
    }
    uimglen_t width = x_ * self->width;
    uimglen_t height = y_ * self->height;
    ScaleFilter scale_filter = scale_filter_from_string(luaL_checkstring(L, 3));
    ImageBase *out = self->scale(width, height, scale_filter);
    push_image(L, out);
    return 1;
HANDLE_END
}

static int image_rotate (lua_State *L)
{
    if (lua_gettop(L) == 2) {
        ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        float angle = luaL_checknumber(L, 2);
        push_image(L, self->rotate(angle, NULL));
    } else {
        check_args(L,3);
        ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        float angle = luaL_checknumber(L, 2);
        ColourBase *colour = alloc_colour(L, self->channels(), self->hasAlpha(), 3);
        push_image(L, self->rotate(angle, colour));
        delete colour;
    }
    return 1;
}

static int image_clone (lua_State *L)
{
    check_args(L, 1);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    ImageBase *out = self->clone(false, false);
    push_image(L, out);
    return 1;
}

static int image_flip (lua_State *L)
{
    check_args(L, 1);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    ImageBase *out = self->clone(false, true);
    push_image(L, out);
    return 1;
}

static int image_mirror (lua_State *L)
{
    check_args(L, 1);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    ImageBase *out = self->clone(true, false);
    push_image(L, out);
    return 1;
}

static int image_mean_diff (lua_State *L)
{
    const ImageBase *some_image;
    ColourBase *value = image_zip_reduce_lua1<op_diff,op_add>(L, some_image);
    float num_pixels = some_image->numPixels();
    float *raw = (float*)value; // maybe UB in C++ (OK in C)
    for (chan_t c=0 ; c<some_image->channels() ; ++c) {
        raw[c] = raw[c]/num_pixels;
    }
    push_colour(L, some_image->channels(), some_image->hasAlpha(), *value);
    delete value;
    return 1;
}

static int image_rms_diff (lua_State *L)
{
    const ImageBase *some_image;
    ColourBase *value = image_zip_reduce_lua1<op_diffsq,op_add>(L, some_image);
    float num_pixels = some_image->numPixels();
    float *raw = (float*)value; // maybe UB in C++ (OK in C)
    for (chan_t c=0 ; c<some_image->channels() ; ++c) {
        raw[c] = raw[c]/num_pixels;
    }
    push_colour(L, some_image->channels(), some_image->hasAlpha(), *value);
    delete value;
    return 1;
}

static int image_abs (lua_State *L)
{
    check_args(L,1);
    ImageBase *src = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    push_image(L, src->abs());
    return 1;
}

static int image_draw (lua_State *L)
{
    check_args(L,3);
    uimglen_t x;
    uimglen_t y;
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    check_coord(L, 2, x, y);
    int pi = 3;

    if (x>=self->width || y>=self->height) {
        std::stringstream ss;
        ss << "Colour coordinates out of range: (" << x << "," << y << ")";
        my_lua_error(L, ss.str());
    }

    ColourBase *colour = NULL;
    if (self->hasAlpha()) {
        colour = alloc_colour(L, self->channels(), true, pi);
    } else {
        colour = alloc_colour(L, self->channels()+1, true, pi);
    }

    self->drawPixelSafe(x, y, colour);

    delete colour;

    return 0;
}

static void draw_image_common (lua_State *L, ImageBase *dst, ImageBase *src, simglen_t x, simglen_t y, bool wrap_x, bool wrap_y)
{

    if (!src->hasAlpha()) {
        my_lua_error(L, "Can only draw images with alpha channels.");
    }

    if (src->colourChannels() != dst->colourChannels()) {
        my_lua_error(L, "Can only draw onto image with same number of colour channels.");
    }

    dst->drawImage(src, x, y, wrap_x, wrap_y);
}

static int image_draw_image_at (lua_State *L)
{
    bool wrap_x = false;
    bool wrap_y = false;
    simglen_t x, y;
    ImageBase *dst, *src;

    if (lua_gettop(L) == 5) {
        dst = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        src = check_ptr<ImageBase>(L, 2, IMAGE_TAG);
        float x_, y_;
        lua_checkvector2(L, 3, &x_, &y_);
        x = x_ - src->width/2;
        y = y_ - src->height/2;
        wrap_x = check_bool(L, 4);
        wrap_y = check_bool(L, 5);
    } else {
        check_args(L,3);
        dst = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        src = check_ptr<ImageBase>(L, 2, IMAGE_TAG);
        float x_, y_;
        lua_checkvector2(L, 3, &x_, &y_);
        x = x_ - src->width/2;
        y = y_ - src->height/2;
    }

    draw_image_common(L, dst, src, x, y, wrap_x, wrap_y);
    return 0;
}

static int image_draw_line (lua_State *L)
{
    check_args(L,5);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    uimglen_t x0, y0;
    uimglen_t x1, y1;
    check_coord(L, 2, x0, y0);
    check_coord(L, 3, x1, y1);
    uimglen_t w = check_t<uimglen_t>(L, 4);
    if (w == 0) my_lua_error(L, "Cannot draw a line with width of 0");
    ColourBase *colour = NULL;
    if (self->hasAlpha()) {
        colour = alloc_colour(L, self->channels(), true, 5);
    } else {
        colour = alloc_colour(L, self->channels()+1, true, 5);
    }
    self->drawLine(x0, y0, x1, y1, w, colour);
    delete colour;
    return 0;
}

static int image_draw_image (lua_State *L)
{
    bool wrap_x = false;
    bool wrap_y = false;
    simglen_t x, y;
    ImageBase *dst, *src;

    if (lua_gettop(L) == 5) {
        dst = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        src = check_ptr<ImageBase>(L, 2, IMAGE_TAG);
        check_scoord(L, 3, x, y);
        wrap_x = check_bool(L, 4);
        wrap_y = check_bool(L, 5);
    } else {
        check_args(L,3);
        dst = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        src = check_ptr<ImageBase>(L, 2, IMAGE_TAG);
        check_scoord(L, 3, x, y);
    }

    draw_image_common(L, dst, src, x, y, wrap_x, wrap_y);
    return 0;
}

static int image_max (lua_State *L)
{
    push_image(L, image_zip_lua1<op_max>(L));
    return 1;
}

static int image_min (lua_State *L)
{
    push_image(L, image_zip_lua1<op_min>(L));
    return 1;
}

static int global_lerp (lua_State *L)
{
    check_args(L,3);
    global_lerp_lua1(L);
    return 1;
}

static int image_convolve (lua_State *L)
{
    bool wrap_x = false;
    bool wrap_y = false;
    switch (lua_gettop(L)) {
        case 4: wrap_y = check_bool(L, 4);
        case 3: wrap_x = check_bool(L, 3);
        case 2: break;
        default: 
        my_lua_error(L, "image_convolve takes 2, 3, or 4 arguments");
    }
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    ImageBase *kernel = check_ptr<ImageBase>(L, 2, IMAGE_TAG);
    if (kernel->channels() != 1) {
        my_lua_error(L, "Convolution kernel must have only 1 channel.");
    }
    if (kernel->hasAlpha()) {
        my_lua_error(L, "Convolution kernel must not have alpha channel.");
    }
    Image<1,0> *kern = static_cast<Image<1,0>*>(kernel);
    if (kernel->width % 2 != 1) {
        my_lua_error(L, "Convolution kernel width must be an odd number.");
    }
    if (kernel->height % 2 != 1) {
        my_lua_error(L, "Convolution kernel height must be an odd number.");
    }
    push_image(L, self->convolve(kern, wrap_x, wrap_y));
    return 1;
}

static int image_convolve_sep (lua_State *L)
{
    bool wrap_x = false;
    bool wrap_y = false;
    switch (lua_gettop(L)) {
        case 4: wrap_y = check_bool(L, 4);
        case 3: wrap_x = check_bool(L, 3);
        case 2: break;
        default: 
        my_lua_error(L, "image_convolve_sep takes 2, 3, or 4 arguments");
    }
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    ImageBase *kernel_x = check_ptr<ImageBase>(L, 2, IMAGE_TAG);
    if (kernel_x->channels() != 1) {
        my_lua_error(L, "Separable convolution kernel must have only 1 channel.");
    }
    if (kernel_x->hasAlpha()) {
        my_lua_error(L, "Convolution kernel must not have alpha channel.");
    }
    Image<1,0> *kern_x = static_cast<Image<1,0>*>(kernel_x);
    if (kern_x->width % 2 != 1) {
        my_lua_error(L, "Separable convolution kernel width must be an odd number.");
    }
    if (kern_x->height != 1) {
        my_lua_error(L, "Separable convolution kernel height must be 1.");
    }
    Image<1,0> *kern_y = kern_x->rotate(90,NULL);
    ImageBase *nu = self->convolve(kern_x, wrap_x, wrap_y);
    ImageBase *nu2 = nu->convolve(kern_y, wrap_x, wrap_y);
    delete nu;
    delete kern_y;
    push_image(L, nu2);
    return 1;
}

static int image_normalise (lua_State *L)
{
    check_args(L,1);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    push_image(L, self->normalise());
    return 1;
}

template<chan_t sch, chan_t scha, chan_t dch, chan_t dcha>
ImageBase *image_swizzle3 (const Image<sch,scha> *src, int *mapping)
{
    Image<dch,dcha> *dst = new Image<dch,dcha>(src->width, src->height);
    for (uimglen_t y=0 ; y<src->height ; ++y) {
        for (uimglen_t x=0 ; x<src->width ; ++x) {
            for (chan_t c=0 ; c<dch+dcha ; ++c) {
                switch (mapping[c]) {
                    case -1:
                    dst->pixel(x,y)[c] = 0;
                    break;
                    case -2:
                    dst->pixel(x,y)[c] = 1;
                    break;
                    default:
                    dst->pixel(x,y)[c] = src->pixel(x,y)[mapping[c]];
                }
            }
        }
    }
    return dst;
}
        
template<chan_t sch, chan_t scha>
static ImageBase *image_swizzle2 (const ImageBase *img_, chan_t ch2, bool a2, int *m)
{
    const Image<sch,scha> *img = static_cast<const Image<sch,scha>*>(img_);
    switch (ch2) {
        case 1: return image_swizzle3<sch,scha,1,0>(img, m);
        case 2: return a2 ? image_swizzle3<sch,scha,1,1>(img, m) : image_swizzle3<sch,scha,2,0>(img, m);
        case 3: return a2 ? image_swizzle3<sch,scha,2,1>(img, m) : image_swizzle3<sch,scha,3,0>(img, m);
        case 4: return a2 ? image_swizzle3<sch,scha,3,1>(img, m) : image_swizzle3<sch,scha,4,0>(img, m);
        default:;
    }
    return NULL;
}

// key guaranteed to be at most 4 letters long and only contain wxyz
static ImageBase *image_swizzle (const ImageBase *img, chan_t ch2, bool a2, int *m)
{
    switch (img->channels()) {
        case 1: return image_swizzle2<1,0>(img, ch2, a2, m);
        case 2: return img->hasAlpha() ? image_swizzle2<1,1>(img, ch2, a2, m) : image_swizzle2<2,0>(img, ch2, a2, m);
        case 3: return img->hasAlpha() ? image_swizzle2<2,1>(img, ch2, a2, m) : image_swizzle2<3,0>(img, ch2, a2, m);
        case 4: return img->hasAlpha() ? image_swizzle2<3,1>(img, ch2, a2, m) : image_swizzle2<4,0>(img, ch2, a2, m);
        default:;
    }
    return NULL;
}

static int image_index (lua_State *L)
{
    check_args(L,2);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    const char *key = luaL_checkstring(L, 2);
    if (!::strcmp(key, "allChannels")) {
        lua_pushnumber(L, self->channels());
    } else if (!::strcmp(key, "colourChannels")) {
        lua_pushnumber(L, self->colourChannels());
    } else if (!::strcmp(key, "hasAlpha")) {
        lua_pushboolean(L, self->hasAlpha());
    } else if (!::strcmp(key, "width")) {
        lua_pushnumber(L, self->width);
    } else if (!::strcmp(key, "height")) {
        lua_pushnumber(L, self->height);
    } else if (!::strcmp(key, "size")) {
        lua_pushvector2(L, self->width, self->height);
    } else if (!::strcmp(key, "numPixels")) {
        lua_pushnumber(L, self->numPixels());
    } else if (!::strcmp(key, "save")) {
        lua_pushcfunction(L, image_save);
    } else if (!::strcmp(key, "foreach")) {
        lua_pushcfunction(L, image_foreach);
    } else if (!::strcmp(key, "map")) {
        lua_pushcfunction(L, image_map);
    } else if (!::strcmp(key, "reduce")) {
        lua_pushcfunction(L, image_reduce);
    } else if (!::strcmp(key, "crop")) {
        lua_pushcfunction(L, image_crop);
    } else if (!::strcmp(key, "cropCentre")) {
        lua_pushcfunction(L, image_crop_centre);
    } else if (!::strcmp(key, "scale")) {
        lua_pushcfunction(L, image_scale);
    } else if (!::strcmp(key, "scaleBy")) {
        lua_pushcfunction(L, image_scale_by);
    } else if (!::strcmp(key, "rotate")) {
        lua_pushcfunction(L, image_rotate);
    } else if (!::strcmp(key, "clone")) {
        lua_pushcfunction(L, image_clone);
    } else if (!::strcmp(key, "flip")) {
        lua_pushcfunction(L, image_flip);
    } else if (!::strcmp(key, "mirror")) {
        lua_pushcfunction(L, image_mirror);
    } else if (!::strcmp(key, "rmsDiff")) {
        lua_pushcfunction(L, image_rms_diff);
    } else if (!::strcmp(key, "meanDiff")) {
        lua_pushcfunction(L, image_mean_diff);
    } else if (!::strcmp(key, "abs")) {
        lua_pushcfunction(L, image_abs);
    } else if (!::strcmp(key, "max")) {
        lua_pushcfunction(L, image_max);
    } else if (!::strcmp(key, "min")) {
        lua_pushcfunction(L, image_min);
    } else if (!::strcmp(key, "convolve")) {
        lua_pushcfunction(L, image_convolve);
    } else if (!::strcmp(key, "convolveSep")) {
        lua_pushcfunction(L, image_convolve_sep);
    } else if (!::strcmp(key, "normalise")) {
        lua_pushcfunction(L, image_normalise);
    } else if (!::strcmp(key, "draw")) {
        lua_pushcfunction(L, image_draw);
    } else if (!::strcmp(key, "drawLine")) {
        lua_pushcfunction(L, image_draw_line);
    } else if (!::strcmp(key, "drawImage")) {
        lua_pushcfunction(L, image_draw_image);
    } else if (!::strcmp(key, "drawImageAt")) {
        lua_pushcfunction(L, image_draw_image_at);
    } else {
        chan_t nu_chans = strlen(key);
        if (nu_chans<=4) {
            bool swizzle = true;
            bool has_alpha = false;
            int mapping[4];
            for (chan_t c=0 ; c<nu_chans ; ++c) {
                int src_chan = 0;
                switch (key[c]) {
                    case 'x': case 'X': src_chan = 0; break;
                    case 'y': case 'Y': src_chan = 1; break;
                    case 'z': case 'Z': src_chan = 2; break;
                    case 'w': case 'W': src_chan = 3; break;
                    case 'e': case 'E': src_chan = -1; break;
                    case 'f': case 'F': src_chan = -2; break;
                    default: swizzle = false;
                }
                mapping[c] = src_chan;
                switch (key[c]) {
                    case 'x': case 'y': case 'z': case 'w': case 'e': case 'f': break;
                    case 'X': case 'Y': case 'Z': case 'W': case 'E': case 'F':
                    if (c != nu_chans-1) {
                        my_lua_error(L, "On image swizzle, only last channel can have alpha: \""+std::string(key)+"\"");
                    }
                    has_alpha = true;
                    break;
                    default:;
                }
                if (src_chan >= self->channels()) {
                    my_lua_error(L, "Image does not have enough channels for swizzle: \""+std::string(key)+"\"");
                }
            }
            if (swizzle) {
                push_image(L, image_swizzle(self, nu_chans, has_alpha, mapping));
                return 1;
            } else {
                my_lua_error(L, "Not a readable Image field: \""+std::string(key)+"\"");
            }
    
        } else {
            my_lua_error(L, "Not a readable Image field: \""+std::string(key)+"\"");
        }
    }
    return 1;
}

static int image_call (lua_State *L)
{
    uimglen_t x;
    uimglen_t y;
    switch (lua_gettop(L)) {
        case 2:
        check_coord(L, 2, x, y);
        break;
        case 3:
        x = check_t<uimglen_t>(L, 2);
        y = check_t<uimglen_t>(L, 3);
        break;
        default:
        my_lua_error(L, "Only allowed: image(x,y) or image(vector2(x,y))");
        return 1;
    }
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);

    if (x>=self->width || y>=self->height) {
        std::stringstream ss;
        ss << "Colour coordinates out of range: (" << x << "," << y << ")";
        my_lua_error(L, ss.str());
    }
    push_colour(L, self->channels(), self->hasAlpha(), self->pixelSlow(x,y));
    return 1;
}

static int image_add (lua_State *L)
{
    push_image(L, image_zip_lua1<op_add>(L));
    return 1;
}

static int image_sub (lua_State *L)
{
    push_image(L, image_zip_lua1<op_sub>(L));
    return 1;
}

static int image_mul (lua_State *L)
{
    push_image(L, image_zip_lua1<op_mul>(L));
    return 1;
}

static int image_div (lua_State *L)
{
    push_image(L, image_zip_lua1<op_div>(L));
    return 1;
}

static int image_pow (lua_State *L)
{
    push_image(L, image_zip_lua1<powf>(L));
    return 1;
}

static int image_concat (lua_State *L)
{
    push_image(L, image_blend_lua1(L));
    return 1;
}

static int image_unm (lua_State *L)
{
    check_args(L,2); // quirk of lua -- takes 2 even though 1 is unused
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    push_image(L, self->unm());
    return 1;
}



const luaL_reg image_meta_table[] = {
    {"__tostring", image_tostring},
    {"__gc",       image_gc},
    {"__index",    image_index},
    {"__eq",       image_eq},
    {"__call",     image_call},
    {"__mul",      image_mul},
    {"__unm",      image_unm}, 
    {"__add",      image_add}, 
    {"__sub",      image_sub}, 
    {"__div",      image_div}, 
    {"__pow",      image_pow},
    {"__concat",   image_concat},

    {NULL, NULL}
};




/*
void push_vimage (lua_State *L, VoxelImage *image)
{
    if (image == NULL) {
        std::cerr << "INTERNAL ERROR: pushing a null image" << std::endl;
        abort();
    }
    void **self_ptr = static_cast<void**>(lua_newuserdata(L, sizeof(*self_ptr)));
    *self_ptr = image;
    luaL_getmetatable(L, VIMAGE_TAG);
    lua_setmetatable(L, -2);
}


static int vimage_gc (lua_State *L)
{ 
    check_args(L, 1); 
    VoxelImage *self = check_ptr<VoxelImage>(L, 1, VIMAGE_TAG);
    delete self; 
    return 0; 
}

static int vimage_eq (lua_State *L)
{
    check_args(L, 2); 
    VoxelImage *self = check_ptr<VoxelImage>(L, 1, VIMAGE_TAG);
    VoxelImage *that = check_ptr<VoxelImage>(L, 2, VIMAGE_TAG);
    lua_pushboolean(L, self==that); 
    return 1; 
}

static int vimage_tostring (lua_State *L)
{
    check_args(L,1);
    VoxelImage *self = check_ptr<VoxelImage>(L, 1, VIMAGE_TAG);
    std::stringstream ss;
    ss << *self;
    push_string(L, ss.str());
    return 1;
}

static int vimage_render (lua_State *L)
{
    check_args(L,3);
    VoxelImage *self = check_ptr<VoxelImage>(L, 1, VIMAGE_TAG);
    uimglen_t width, height;
    check_coord(L, 2, width, height);
    float x,y,z;
    lua_checkvector3(L, 3, &x, &y, &z);

    Image<3> *rendered = new Image<3>(width, height);

    self->render(rendered, x, y, z);

    push_image(L, rendered);
    return 1;
}

static int vimage_index (lua_State *L)
{
    check_args(L,2);
    VoxelImage *self = check_ptr<VoxelImage>(L, 1, VIMAGE_TAG);
    const char *key = luaL_checkstring(L, 2);
    if (!::strcmp(key, "width")) {
        lua_pushnumber(L, self->width);
    } else if (!::strcmp(key, "height")) {
        lua_pushnumber(L, self->height);
    } else if (!::strcmp(key, "depth")) {
        lua_pushnumber(L, self->depth);
    } else if (!::strcmp(key, "size")) {
        lua_pushvector3(L, self->width, self->height, self->depth);
    } else if (!::strcmp(key, "render")) {
        lua_pushcfunction(L, vimage_render);
    } else {
        my_lua_error(L, "Not a readable VoxelImage field: \""+std::string(key)+"\"");
    }
    return 1;
}

const luaL_reg vimage_meta_table[] = {
    {"__tostring", vimage_tostring},
    {"__gc",       vimage_gc},
    {"__index",    vimage_index},
    {"__eq",       vimage_eq},

    {NULL, NULL}
};
*/



template<chan_t ch, chan_t ach>
ImageBase *image_from_lua_func (lua_State *L, uimglen_t width, uimglen_t height, int func_index)
{
    Image<ch,ach> *my_image = new Image<ch,ach>(width, height);
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            lua_pushvalue(L, func_index);
            lua_pushvector2(L, x, y);
            int status = lua_pcall(L, 1, 1, 0); 
            if (status == 0) {
                Colour<ch, ach> p;
                if (!check_colour(L, p, -1)) {
                    delete my_image;
                    my_lua_error(L, "While initialising the image at ("+str(x)+","+str(y)+"): "
                                    "returned value had bad type: "+type_name(L,-1));
                } else {
                    my_image->pixel(x,y) = p;
                }
            } else {
                const char *msg = lua_tostring(L, -1);
                delete my_image;
                my_lua_error(L, "While initialising the image at ("+str(x)+","+str(y)+"): "+str(msg));
            }
            lua_pop(L, 1);
        }   
    }   
    return my_image;
}

template<chan_t ch, chan_t ach>
ImageBase *image_from_lua_table (lua_State *L, uimglen_t width, uimglen_t height, int tab_index)
{
    unsigned int elements = luaL_getn(L, tab_index);
    if (elements != width * height)
        my_lua_error(L, "Initialisation table for image "+str(width)+"x"+str(height)+" has "+str(elements)+" elements.");

    Image<ch,ach> *my_image = new Image<ch,ach>(width, height);
    Colour<ch,ach> p;
    for (uimglen_t y=0 ; y<height ; ++y) {
        for (uimglen_t x=0 ; x<width ; ++x) {
            lua_rawgeti(L, tab_index, y*width+x+1);
            if (!check_colour(L, p, -1)) {
                delete my_image;
                my_lua_error(L, "While initialising the image at ("+str(x)+","+str(y)+"): "
                                "initialisation table contained bad type: "+type_name(L,-1));
            } else {
                my_image->pixel(x,y) = p;
            }
            lua_pop(L, 1);
        }   
    }   
    return my_image;
}

uimglen_t fact (uimglen_t x)
{
    uimglen_t counter = 1;
    for (uimglen_t i=2 ; i<=x ; ++i) {
        counter *= i;
    }
    return counter;
}

static int global_gaussian (lua_State *L)
{
    check_args(L,1);
    uimglen_t size = check_t<uimglen_t>(L, 1);
    Image<1,0> *my_image = new Image<1,0>(size, 1);
    for (uimglen_t x=0 ; x<size ; ++x) {
        my_image->pixel(x,0) = fact(size-1) / (fact(x) * fact(size-1-x));
    }
    push_image(L, my_image->normalise());
    delete my_image;
    return 1;
}

static int global_make (lua_State *L)
{
    uimglen_t w, h;
    chan_t channels;
    bool alpha = false;
    int ii;
    if (lua_gettop(L) == 4) {
        check_coord(L, 1, w, h);
        channels = check_int(L, 2, 1, 4);
        alpha = check_bool(L, 3);
        if (channels==4 && alpha) my_lua_error(L, "Image with alpha channel can have at most 3 colour channels.");
        ii = 4;
    } else {
        check_args(L,3);
        check_coord(L, 1, w, h);
        channels = check_int(L, 2, 1, 4);
        ii = 3;
    }
    if (alpha) channels++;

    ImageBase *image = NULL;

    switch (lua_type(L, ii)) {
        case LUA_TFUNCTION: {
            switch (channels) {
                case 1: image = image_from_lua_func<1,0>(L,w,h,ii); break;
                case 2: image = alpha ? image_from_lua_func<1,1>(L,w,h,ii) : image_from_lua_func<2,0>(L,w,h,ii); break;
                case 3: image = alpha ? image_from_lua_func<2,1>(L,w,h,ii) : image_from_lua_func<3,0>(L,w,h,ii); break;
                case 4: image = alpha ? image_from_lua_func<3,1>(L,w,h,ii) : image_from_lua_func<4,0>(L,w,h,ii); break;
                default: my_lua_error(L, "Internal error");
            }
        }
        break;
        case LUA_TTABLE: {
            switch (channels) {
                case 1: image = image_from_lua_table<1,0>(L,w,h,ii); break;
                case 2: image = alpha ? image_from_lua_table<1,1>(L,w,h,ii) : image_from_lua_table<2,0>(L,w,h,ii); break;
                case 3: image = alpha ? image_from_lua_table<2,1>(L,w,h,ii) : image_from_lua_table<3,0>(L,w,h,ii); break;
                case 4: image = alpha ? image_from_lua_table<3,1>(L,w,h,ii) : image_from_lua_table<4,0>(L,w,h,ii); break;
                default: my_lua_error(L, "Internal error");
            }
        }
        break;
        default:
        if (get_colour_channels(L,ii)==0)
            my_lua_error(L, "Expected a number, vector, table, or function to initialise image");
        ColourBase *init = alloc_colour(L, channels, alpha, ii);
        switch (channels) {
            case 1: image = image_make_base<1,0>(w,h,*init); break;
            case 2: image = alpha ? image_make_base<1,1>(w,h,*init) : image_make<2,0>(w,h,*init); break;
            case 3: image = alpha ? image_make_base<2,1>(w,h,*init) : image_make<3,0>(w,h,*init); break;
            case 4: image = alpha ? image_make_base<3,1>(w,h,*init) : image_make<4,0>(w,h,*init); break;
            default: my_lua_error(L, "Internal error");
        }
        delete init;
    }

    push_image(L, image);
    return 1;
}

static int global_open (lua_State *L)
{
HANDLE_BEGIN
    check_args(L,1);
    std::string filename = luaL_checkstring(L,1);
    ImageBase *image = image_load(filename);
    if (image == NULL) {
        lua_pushnil(L);
    } else {
        push_image(L, image);
    }
    return 1;
HANDLE_END
}

static int global_text_codepoint (lua_State *L)
{
HANDLE_BEGIN
    check_args(L,3);
    std::string font = luaL_checkstring(L,1);
    uimglen_t width, height;
    check_coord(L, 2, width, height);
    std::string text = luaL_checkstring(L,3);
    size_t i = 0;
    unsigned long cp = decode_utf8(text, i);
    if (i != text.size()-1) my_lua_error(L, "Only one character can be supplied to text_codepoint().");
    ImageBase *image = make_text_codepoint(font, width, height, cp);
    push_image(L, image);
    return 1;
HANDLE_END
}

static int global_text (lua_State *L)
{
HANDLE_BEGIN
    if (lua_gettop(L) == 5) {
        std::string font = luaL_checkstring(L,1);
        uimglen_t width, height;
        check_coord(L, 2, width, height);
        std::string text = luaL_checkstring(L,3);
        float xx, xy, yx, yy;
        lua_checkvector2(L, 4, &xx, &xy);
        lua_checkvector2(L, 5, &yx, &yy);
        ImageBase *image = make_text(font, width, height, text, xx, xy, yx, yy);
        push_image(L, image);
    } else {
        check_args(L,3);
        std::string font = luaL_checkstring(L,1);
        uimglen_t width, height;
        check_coord(L, 2, width, height);
        std::string text = luaL_checkstring(L,3);
        ImageBase *image = make_text(font, width, height, text, 1, 0, 0, 1);
        push_image(L, image);
    }
    return 1;
HANDLE_END
}

static int global_mipmaps (lua_State *L)
{
HANDLE_BEGIN
    ImageBase *self;
    ScaleFilter scale_filter = SF_BOX;
    switch (lua_gettop(L)) {
        case 2:
        scale_filter = scale_filter_from_string(luaL_checkstring(L, 2));
        case 1:
        self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
        break;
        default: my_lua_error(L, "Expected 1 or 2 args.");
    }
    unsigned counter = 1;
    lua_newtable(L);
    int table_index = lua_gettop(L);

    lua_pushvalue(L, 1);
    ImageBase *last = self;
    uimglen_t width = last->width;
    uimglen_t height = last->height;
    lua_rawseti(L, table_index, counter++);

    do {
        width = last->width == 1 ? 1 : last->width / 2;
        height = last->height == 1 ? 1 : last->height / 2;
        last = last->scale(width, height, scale_filter);
        push_image(L, last);
        lua_rawseti(L, table_index, counter++);
    } while (height > 1 || width > 1);

    return 1;
HANDLE_END
}

static int get_squish_flags (lua_State *L, int tab)
{
    unsigned args = lua_gettop(L);
    int quality = SQUISH_QUALITY_HIGH;
    int metric = SQUISH_METRIC_PERCEPTUAL;
    int alpha_weight = 0;
    for (unsigned i=tab ; i<=args ; ++i) {
        std::string flag = luaL_checkstring(L, i);
        if (flag == "QUALITY_HIGHEST") {
            quality = SQUISH_QUALITY_HIGHEST;
        } else if (flag == "QUALITY_HIGH") {
            quality = SQUISH_QUALITY_HIGH;
        } else if (flag == "QUALITY_LOW") {
            quality = SQUISH_QUALITY_LOW;
        } else if (flag == "METRIC_PERCEPTUAL") {
            metric = SQUISH_METRIC_PERCEPTUAL;
        } else if (flag == "METRIC_UNIFORM") {
            metric = 0;
        } else if (flag == "WEIGHT_COLOUR_BY_ALPHA") {
            alpha_weight = SQUISH_WEIGHT_COLOUR_BY_ALPHA;
        } else if (flag == "NO_WEIGHT_COLOUR_BY_ALPHA") {
            alpha_weight = 0;
        } else {
            EXCEPT << "Unrecognised DDS flag: " << flag << ENDL;
        }
    }
    return quality | metric | alpha_weight;
}

static ImageBases get_image_vector (lua_State *L, int table_index)
{
    ImageBases imgs;
    // pull mips out of table
    if (lua_istable(L, table_index)) {
        int counter = 1;
        while (true) {
            lua_rawgeti(L, table_index, counter);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                break;
            }
            imgs.push_back(check_ptr<ImageBase>(L, -1, IMAGE_TAG));
            lua_pop(L, 1);
            counter++;
        }
    } else {
        imgs.push_back(check_ptr<ImageBase>(L, table_index, IMAGE_TAG));
    }
    if (imgs.size() == 0) {
        my_lua_error(L, "Table had no elements.");
    }
    return imgs;
}

static bool is_power_of_two (uimglen_t x)
{
    return !((x-1) & x);
}

template<chan_t ch, chan_t ach> Image<ch,ach> *volume_box (const ImageBase *top_, const ImageBase *bot_, uimglen_t w, uimglen_t h)
{
    auto top = static_cast<const Image<ch,ach> *>(top_);
    auto bot = static_cast<const Image<ch,ach> *>(bot_);
    auto r = new Image<ch,ach>(w, h);
    for (uimglen_t y=0 ; y<h ; ++y) {
        for (uimglen_t x=0 ; x<w ; ++x) {
            Colour<ch, ach> col(0);
            for (chan_t c=0 ; c<ch+ach ; ++c) {
                col[c] += top->pixel(2*x,2*y)[c] / 8;
                col[c] += top->pixel(2*x,2*y+1)[c] / 8;
                col[c] += top->pixel(2*x+1,2*y)[c] / 8;
                col[c] += top->pixel(2*x+1,2*y+1)[c] / 8;
                col[c] += bot->pixel(2*x,2*y)[c] / 8;
                col[c] += bot->pixel(2*x,2*y+1)[c] / 8;
                col[c] += bot->pixel(2*x+1,2*y)[c] / 8;
                col[c] += bot->pixel(2*x+1,2*y+1)[c] / 8;
            }
            r->pixel(x,y) = col;
        }
    }
    return r;
}

static int global_volume_mipmaps (lua_State *L)
{
HANDLE_BEGIN
    check_args(L, 1);
    static const char *msg = "Argument must be a non-empty array of same size/channels/alpha images (a volume map).";
    if (!lua_istable(L, 1)) my_lua_error(L, msg);
    ImageBases volume = get_image_vector(L, 1);
    uimglen_t vol_depth = volume.size();
    if (vol_depth == 0) my_lua_error(L, "volume_mipmaps received an empty array.");
    uimglen_t vol_width = volume[0]->width;
    uimglen_t vol_height = volume[0]->height;
    unsigned ch = volume[0]->colourChannels();
    bool ach = volume[0]->hasAlpha();

    // verify
    for (unsigned i=1 ; i<vol_depth ; ++i) {
        if (volume[i]->width != vol_width || volume[i]->height != vol_height)
            my_lua_error(L, msg);
        if (volume[i]->colourChannels() != ch || volume[i]->hasAlpha() != ach)
            my_lua_error(L, msg);
    }

    if (!is_power_of_two(vol_depth) || !is_power_of_two(vol_width) || !is_power_of_two(vol_height)) {
        my_lua_error(L, "Currently volume_mipmaps only supports power of 2 textures.");
    }

    unsigned mip = 0;

    lua_newtable(L);
    int mips_table_index = lua_gettop(L);

    // push top mipmap level into new table, but reuse original image userdatas.
    lua_newtable(L);
    for (uimglen_t z=0 ; z<vol_depth ; ++z) {
        lua_rawgeti(L, 1, z+1);
        lua_rawseti(L, -2, z+1);
    }
    lua_rawseti(L, mips_table_index, mip+1);
    mip++;

    ImageBases last_volume = volume;
    uimglen_t mip_width;
    uimglen_t mip_height;
    uimglen_t mip_depth;
    do {
        // make a vector of images
        // initialise them
        // push all layers into a table and set into top table
        mip_width = vol_width>>mip > 0 ? vol_width>>mip : 1;
        mip_height = vol_height>>mip > 0 ? vol_height>>mip : 1;
        mip_depth = vol_depth>>mip > 0 ? vol_depth>>mip : 1;
        lua_newtable(L);
        ImageBases new_volume;
        for (unsigned z=0 ; z<mip_depth ; ++z) {
            ImageBase *top = last_volume[z*2 + 0];
            ImageBase *bot = last_volume.size()==1 ? top : last_volume[z*2 + 1];
            ImageBase *layer = NULL;
            switch (ch) {
                case 1:
                if (ach) {
                    layer = volume_box<1,1>(top, bot, mip_width, mip_height);
                } else {
                    layer = volume_box<1,0>(top, bot, mip_width, mip_height);
                }
                break;
                case 2:
                if (ach) {
                    layer = volume_box<2,1>(top, bot, mip_width, mip_height);
                } else {
                    layer = volume_box<2,0>(top, bot, mip_width, mip_height);
                }
                break;
                case 3:
                if (ach) {
                    layer = volume_box<3,1>(top, bot, mip_width, mip_height);
                } else {
                    layer = volume_box<3,0>(top, bot, mip_width, mip_height);
                }
                break;
                case 4:
                if (ach) {
                    layer = volume_box<4,1>(top, bot, mip_width, mip_height);
                } else {
                    layer = volume_box<4,0>(top, bot, mip_width, mip_height);
                }
                break;
                default: EXCEPTEX << ch << ENDL;
            }
            new_volume.push_back(layer);
        }
        for (unsigned z=0 ; z<mip_depth ; ++z) {
            push_image(L, new_volume[z]);
            lua_rawseti(L, -2, z+1);
        }
        lua_rawseti(L, mips_table_index, mip+1);
        last_volume = new_volume;
        mip++;
    } while (mip_width>1 || mip_height>1 || mip_depth>1);

    return 1;
HANDLE_END
}

static int global_dds_save_simple (lua_State *L)
{
HANDLE_BEGIN
    unsigned args = lua_gettop(L);
    if (args < 3) {
        my_lua_error(L, "Expected at least 3 args to dds_save_simple.");
    }
    std::string filename = luaL_checkstring(L, 1);
    DDSFormat format = format_from_string(luaL_checkstring(L, 2));
    DDSFile content;
    content.kind = DDS_SIMPLE;
    content.simple = get_image_vector(L, 3);
    int squish_flags = get_squish_flags(L, 4);
    dds_save(filename, format, content, squish_flags);
    return 0;
HANDLE_END
}

static int global_dds_save_cube (lua_State *L)
{
HANDLE_BEGIN
    unsigned args = lua_gettop(L);
    if (args < 8) {
        my_lua_error(L, "Expected at least 8 args to dds_save_cube.");
    }
    std::string filename = luaL_checkstring(L, 1);
    DDSFormat format = format_from_string(luaL_checkstring(L, 2));
    DDSFile content;
    content.kind = DDS_CUBE;
    content.cube.X = get_image_vector(L, 3);
    content.cube.x = get_image_vector(L, 4);
    content.cube.Y = get_image_vector(L, 5);
    content.cube.y = get_image_vector(L, 6);
    content.cube.Z = get_image_vector(L, 7);
    content.cube.z = get_image_vector(L, 8);
    int squish_flags = get_squish_flags(L, 9);
    dds_save(filename, format, content, squish_flags);
    return 0;
HANDLE_END
}

static int global_dds_save_volume (lua_State *L)
{
HANDLE_BEGIN
    unsigned args = lua_gettop(L);
    if (args < 3) {
        my_lua_error(L, "Expected at least 3 args to dds_save_cube.");
    }
    std::string filename = luaL_checkstring(L, 1);
    DDSFormat format = format_from_string(luaL_checkstring(L, 2));
    int table_index = 3;
    int squish_flags = get_squish_flags(L, 4);
    DDSFile content;
    content.kind = DDS_VOLUME;
    if (!lua_istable(L, table_index)) {
        my_lua_error(L, "3rd parameter must be a table of tables of images.");
    }
    // each mip is a volume
    int counter = 1;
    while (true) {
        lua_rawgeti(L, table_index, counter);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            break;
        }
        if (lua_istable(L, -1)) {
            content.volume.push_back(get_image_vector(L, -1));
        } else {
            my_lua_error(L, "3rd parameter should be a table of tables of images when saving "+filename+".");
        }
        lua_pop(L, 1);
        counter++;
    }
    if (content.volume.size() == 0) {
        my_lua_error(L, "Expected at least one mipmap when saving "+filename+".");
    }
    dds_save(filename, format, content, squish_flags);
    return 0;
HANDLE_END
}

static void push_mipmaps (lua_State *L, const ImageBases &imgs)
{
    lua_newtable(L);
    int table_index = lua_gettop(L);
    for (unsigned i=0 ; i<imgs.size() ; ++i) {
        push_image(L, imgs[i]);
        lua_rawseti(L, table_index, i+1);
    }
}

static int global_dds_open (lua_State *L)
{
HANDLE_BEGIN
    check_args(L,1);
    const char *filename = luaL_checkstring(L, 1);
    DDSFile file = dds_open(filename);
    switch (file.kind) {
        case DDS_SIMPLE: {
            push_mipmaps(L, file.simple);
            lua_pushstring(L, "SIMPLE");
        } break;
        case DDS_CUBE: {
            lua_newtable(L);
            int table_index = lua_gettop(L);
            push_mipmaps(L, file.cube.X);
            lua_setfield(L, table_index, "px");
            push_mipmaps(L, file.cube.x);
            lua_setfield(L, table_index, "nx");
            push_mipmaps(L, file.cube.Y);
            lua_setfield(L, table_index, "py");
            push_mipmaps(L, file.cube.y);
            lua_setfield(L, table_index, "ny");
            push_mipmaps(L, file.cube.Z);
            lua_setfield(L, table_index, "pz");
            push_mipmaps(L, file.cube.z);
            lua_setfield(L, table_index, "nz");
            lua_pushstring(L, "CUBE");
        } break;
        case DDS_VOLUME: {
            lua_newtable(L);
            int mips_table_index = lua_gettop(L);
            unsigned num_mips = file.volume.size();
            for (unsigned i=0 ; i<num_mips ; ++i) {
                lua_newtable(L);
                int layer_table_index = lua_gettop(L);
                for (unsigned l=0 ; l<file.volume[i].size() ; ++l) {
                    push_image(L, file.volume[i][l]);
                    lua_rawseti(L, layer_table_index, l+1);
                }
                lua_rawseti(L, mips_table_index, i+1);
            }
            lua_pushstring(L, "VOLUME");
        } break;
        default:
        EXCEPTEX << file.kind << ENDL;
    }
    return 2;
HANDLE_END
}

static int global_rgb_to_hsl (lua_State *L)
{
    check_args(L,1);
    float r,g,b;
    lua_checkvector3(L, 1, &r, &g, &b);
    float h,s,l;
    RGBtoHSL(r,g,b, h,s,l);
    lua_pushvector3(L, h,s,l);
    return 1;
}

static int global_hsl_to_rgb (lua_State *L)
{
    check_args(L,1);
    float h,s,l;
    lua_checkvector3(L, 1, &h, &s, &l);
    float r,g,b;
    HSLtoRGB(h,s,l, r,g,b);
    lua_pushvector3(L, r,g,b);
    return 1;
}

static int global_hsv_to_hsl (lua_State *L)
{
    check_args(L,1);
    float hh,ss,ll;
    lua_checkvector3(L, 1, &hh, &ss, &ll);
    float h,s,l;
    HSVtoHSL(hh,ss,ll, h,s,l);
    lua_pushvector3(L, h,s,l);
    return 1;
}

static int global_hsl_to_hsv (lua_State *L)
{
    check_args(L,1);
    float h,s,l;
    lua_checkvector3(L, 1, &h, &s, &l);
    float hh,ss,ll;
    HSLtoHSV(h,s,l, hh,ss,ll);
    lua_pushvector3(L, hh,ss,ll);
    return 1;
}

static int global_rgb_to_hsv (lua_State *L)
{
    check_args(L,1);
    float r,g,b;
    lua_checkvector3(L, 1, &r, &g, &b);
    float h,s,v;
    RGBtoHSV(r,g,b, h,s,v);
    lua_pushvector3(L, h,s,v);
    return 1;
}

static int global_hsv_to_rgb (lua_State *L)
{
    check_args(L,1);
    float h,s,v;
    lua_checkvector3(L, 1, &h, &s, &v);
    float r,g,b;
    HSVtoRGB(h,s,v, r,g,b);
    lua_pushvector3(L, r,g,b);
    return 1;
}

static int global_colour (lua_State *L)
{
    check_args(L,2);
    chan_t channels = check_int(L, 1, 1, 4);
    lua_Number f = luaL_checknumber(L, 2);
    switch (channels) {
        case 1: lua_pushnumber(L, f); break;
        case 2: lua_pushvector2(L, f, f); break;
        case 3: lua_pushvector3(L, f, f, f); break;
        case 4: lua_pushvector4(L, f, f, f, f); break;
        default: my_lua_error(L, "Internal error: weird number of channels.");
    }
    return 1;
}

static int global_seconds (lua_State *L)
{
    check_args(L,0);
    lua_pushnumber(L, micros() / 1E6);
    return 1;
}

/*
static int global_make_voxel (lua_State *L)
{
    check_args(L,2);

    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    uimglen_t depth = check_t<uimglen_t>(L, 2);
    

    uimglen_t real_height = self->height / depth;
    if (real_height * depth != self->height) {
        my_lua_error(L, "Input image must have dimensions W*H where H=cube_height*cube_depth");
    }
    VoxelImage *vi = new VoxelImage(self->pixelSlow(0,0).raw(), self->channels(), self->width, real_height, depth, true);

    push_vimage(L, vi);
    return 1;
}
*/

static const luaL_reg global[] = {
    {"make", global_make},
    {"open", global_open},
    {"text_codepoint", global_text_codepoint},
    {"text", global_text},
    {"dds_save_simple", global_dds_save_simple},
    {"dds_save_cube", global_dds_save_cube},
    {"dds_save_volume", global_dds_save_volume},
    {"dds_open", global_dds_open},
    {"mipmaps", global_mipmaps},
    {"volume_mipmaps", global_volume_mipmaps},
    {"RGBtoHSL", global_rgb_to_hsl},
    {"HSLtoRGB", global_hsl_to_rgb},
    {"HSVtoHSL", global_hsv_to_hsl},
    {"HSLtoHSV", global_hsl_to_hsv},
    {"RGBtoHSV", global_rgb_to_hsv},
    {"HSVtoRGB", global_hsv_to_rgb},
    {"lerp", global_lerp},
    {"colour", global_colour},
    {"gaussian", global_gaussian},
    {"seconds", global_seconds},
 //   {"make_voxel", global_make_voxel},

    {NULL, NULL}
};


void lua_wrappers_image_init (lua_State *L)
{
    luaL_newmetatable(L, IMAGE_TAG);
    luaL_register(L, NULL, image_meta_table);
    lua_pop(L,1);

/*
    luaL_newmetatable(L, VIMAGE_TAG);
    luaL_register(L, NULL, vimage_meta_table);
    lua_pop(L,1);
*/

    luaL_register(L, "_G", global);
    lua_pop(L, 1);
}

void lua_wrappers_image_shutdown (lua_State *L)
{
    (void) L;
}
