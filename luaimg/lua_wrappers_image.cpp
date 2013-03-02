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

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

#include "lua_wrappers_image.h"

#include "Image.h"

static void push_string (lua_State *L, const std::string &str) { lua_pushstring(L, str.c_str()); }

static void my_lua_error (lua_State *L, const std::string &msg, unsigned long level=1)
{
    luaL_where(L,level);
    std::string str = lua_tostring(L,-1);
    lua_pop(L,1);
    str += msg;
    push_string(L,str);
    lua_error(L);
}


void check_args (lua_State *L, int expected)
{
    int got = lua_gettop(L);
    if (got != expected) {
        std::stringstream msg;
        msg << "Wrong number of arguments: " << got
            << " should be " << expected;
        my_lua_error(L,msg.str());
    }
}

void check_is_function (lua_State *L, int index)
{
    if (lua_type(L, index) != LUA_TFUNCTION) {
        std::stringstream msg;
        msg << "Expected a function at argument " << index;
        my_lua_error(L,msg.str());
    }
}

template<class T> T* check_ptr (lua_State *L, int index, const char *tag)
{
    return *static_cast<T**>(luaL_checkudata(L, index, tag));
}

template<class T> T* is_ptr (lua_State *L, int index, const char *tag)
{
    return *static_cast<T**>(luaL_checkudata(L, index, tag));
}


lua_Number check_int (lua_State *l, int stack_index,
              lua_Number min, lua_Number max)
{
    lua_Number n = luaL_checknumber(l, stack_index);
    if (n<min || n>max || n!=floor(n)) {
        std::stringstream msg;
        msg << "Not an integer ["<<min<<","<<max<<"]: " << n;
        my_lua_error(l,msg.str());
    }
    return n;
}

template <typename T>
T check_t (lua_State *l, int stack_index,
           T min = std::numeric_limits<T>::min(),
           T max = std::numeric_limits<T>::max())
{
    return (T) check_int(l, stack_index, min, max);
}

static void check_coord (lua_State *L, int index, unsigned &x, unsigned &y)
{
    float x_, y_;
    lua_checkvector2(L, index, &x_, &y_);
    x = x_ < 0 ? 0 : x_ > 65535 ? 65535 : x_;
    y = y_ < 0 ? 0 : y_ > 65535 ? 65535 : y_;
}

template<unsigned ch> bool check_pixel (lua_State *L, Pixel<ch> &p, int index);

template<> bool check_pixel<1> (lua_State *L, Pixel<1> &p, int index)
{
    if (lua_type(L,index) != LUA_TNUMBER) return false;
    p[0] = lua_tonumber(L, index);
    return true;
}

template<> bool check_pixel<2> (lua_State *L, Pixel<2> &p, int index)
{
    if (lua_type(L,index) != LUA_TVECTOR2) return false;
    lua_checkvector2(L, index, &p[0], &p[1]);
    return true;
}

template<> bool check_pixel<3> (lua_State *L, Pixel<3> &p, int index)
{
    if (lua_type(L,index) != LUA_TVECTOR3) return false;
    lua_checkvector3(L, index, &p[0], &p[1], &p[2]);
    return true;
}

/*
template<> bool check_pixel<4> (lua_State *L, Pixel<4> &p, int index)
{
    if (lua_type(L,-1) != LUA_TVECTOR3) return false;
    lua_checkvector4(L, -1, &arr[0], &arr[1], &arr[2], &arr[3]);
    return true;
}
*/


template<unsigned ch> void push_pixel (lua_State *L, Pixel<ch> &p);

template<> void push_pixel<1> (lua_State *L, Pixel<1> &p)
{
    lua_pushnumber(L, p[0]);
}

template<> void push_pixel<2> (lua_State *L, Pixel<2> &p)
{
    lua_pushvector2(L, p[0], p[1]);
}

template<> void push_pixel<3> (lua_State *L, Pixel<3> &p)
{
    lua_pushvector3(L, p[0], p[1], p[2]);
}

/*
template<> void push_pixel<4> (lua_State *L, Pixel<4> &p);
{
    lua_pushvector4(L, p[0], p[1], p[2], p[4]);
}
*/






void push_image (lua_State *L, ImageBase *image)
{
    if (image == NULL) {
        std::cerr << "INTERNAL ERROR: pushing a null image" << std::endl;
        abort();
    }
    void **self_ptr = static_cast<void**>(lua_newuserdata(L, sizeof(*self_ptr)));
    *self_ptr = image;
    luaL_getmetatable(L, IMAGE_TAG);
    lua_setmetatable(L, -2);
}


static int image_gc (lua_State *L)
{ 
    check_args(L, 1); 
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
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
    ss << *self;
    push_string(L, ss.str());
    return 1;
}

static int image_save (lua_State *L)
{
    check_args(L,2);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    std::string filename = lua_tostring(L, 2);
    if (!image_save(self, filename)) my_lua_error(L, "Could not save: \""+filename+"\"");
    return 0;
}

template<unsigned src_ch, unsigned dst_ch> Image<dst_ch> *map_with_lua_func (lua_State *L, ImageBase *src_, int func_index)
{
    Image<src_ch> *src = static_cast<Image<src_ch>*>(src_);
    unsigned width = src->width;
    unsigned height = src->height;
    Image<dst_ch> *dst = new Image<dst_ch>(width, height);
    Pixel<dst_ch> p(0);
    for (unsigned y=0 ; y<height ; ++y) {
        for (unsigned x=0 ; x<width ; ++x) {
            lua_pushvalue(L, func_index);
            push_pixel<src_ch>(L, src->pixel(x,y));
            lua_pushvector2(L, x, y);
            int status = lua_pcall(L, 2, 1, 0); 
            if (status == 0) {
                if (!check_pixel<dst_ch>(L, p, -1)) {
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
    check_args(L,3);
    // img, channels, func
    ImageBase *src = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    unsigned dst_ch = check_int(L, 2, 1, 4);
    check_is_function(L, 3);
    unsigned src_ch = src->channels();
    ImageBase *out = NULL;

    switch (src_ch) {
        case 1:
        switch (dst_ch) {
            case 1:
            out = map_with_lua_func<1,1>(L, src, 3);
            break;

            case 2:
            out = map_with_lua_func<1,2>(L, src, 3);
            break;

            case 3:
            out = map_with_lua_func<1,3>(L, src, 3);
            break;

            case 4:
            //out = map_with_lua_func<1,4>(L, src, 3);
            //break;

            default:
            my_lua_error(L, "Dest channels must be either 1, 2, 3, or 4.");
        }
        break;

        case 2:
        switch (dst_ch) {
            case 1:
            out = map_with_lua_func<2,1>(L, src, 3);
            break;

            case 2:
            out = map_with_lua_func<2,2>(L, src, 3);
            break;

            case 3:
            out = map_with_lua_func<2,3>(L, src, 3);
            break;

            case 4:
            //out = map_with_lua_func<3,4>(L, src, 3);
            //break;

            default:
            my_lua_error(L, "Dest channels must be either 1, 2, 3, or 4.");
        }
        break;

        case 3:
        switch (dst_ch) {
            case 1:
            out = map_with_lua_func<3,1>(L, src, 3);
            break;

            case 2:
            out = map_with_lua_func<3,2>(L, src, 3);
            break;

            case 3:
            out = map_with_lua_func<3,3>(L, src, 3);
            break;

            case 4:
            //out = map_with_lua_func<3,4>(L, src, 3);
            //break;

            default:
            my_lua_error(L, "Dest channels must be either 1, 2, 3, or 4.");
        }
        break;

        default:
        my_lua_error(L, "Source channels must be either 1, 2, 3, or 4.");
    }

    push_image(L, out);
    return 1;
}

template<unsigned ch> void reduce_with_lua_func (lua_State *L, ImageBase *self_, Pixel<ch> zero, int func_index)
{
    Image<ch> *self = static_cast<Image<ch>*>(self_);

    for (unsigned y=0 ; y<self->height ; ++y) {
        for (unsigned x=0 ; x<self->width ; ++x) {
            lua_pushvalue(L, func_index);
            push_pixel<ch>(L, zero);
            push_pixel<ch>(L, self->pixel(x,y));
            lua_pushvector2(L, x, y);
            int status = lua_pcall(L, 3, 1, 0); 
            if (status == 0) {
                if (!check_pixel<ch>(L, zero, -1)) {
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
    push_pixel(L, zero);
}

static int image_reduce (lua_State *L)
{
    check_args(L,3);
    // img:A, zero:A, func:A,A -> A
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    check_is_function(L, 3);
    switch (self->channels()) {
        case 1: {
            Pixel<1> p;
            check_pixel(L, p, 2);
            reduce_with_lua_func(L, self, p, 3);
        }
        break;

        case 2: {
            Pixel<2> p;
            check_pixel(L, p, 2);
            reduce_with_lua_func(L, self, p, 3);
        }
        break;

        case 3: {
            Pixel<3> p;
            check_pixel(L, p, 2);
            reduce_with_lua_func(L, self, p, 3);
        }
        break;

        case 4: {
            //Pixel<4> p;
            //check_pixel(L, p, 2);
            //reduce_with_lua_func(L, self, p, 3);
        }
        //break;


        default:
        my_lua_error(L, "Image must have either 1, 2, 3, or 4 channels.");
    }
    return 1;
}

static int image_crop (lua_State *L)
{
    check_args(L, 3);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    unsigned left, bottom, width, height;
    check_coord(L, 2, left, bottom);
    check_coord(L, 3, width, height);
    if (bottom+height >= self->height || left+width >= self->width) {
        my_lua_error(L, "Crop dimensions out of range of image.");
    }
    ImageBase *out = self->crop(left, bottom, width, height);
    push_image(L, out);
    return 1;
}

static int image_clone (lua_State *L)
{
    check_args(L, 1);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    ImageBase *out = self->crop(0, 0, self->width, self->height);
    push_image(L, out);
    return 1;
}

static int image_rms (lua_State *L)
{
    check_args(L,2);
    // img, float
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    ImageBase *other = check_ptr<ImageBase>(L, 2, IMAGE_TAG);
    lua_pushnumber(L, self->rms(other));
    return 1;
}

static int image_pow (lua_State *L)
{
    check_args(L,2);
    // img, float
    ImageBase *src = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    float index = luaL_checknumber(L, 2);
    ImageBase *out = src->pow(index);
    push_image(L, out);
    return 1;
}

static int image_index (lua_State *L)
{
    check_args(L,2);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    const char *key = luaL_checkstring(L, 2);
    if (!::strcmp(key, "channels")) {
        lua_pushnumber(L, self->channels());
    } else if (!::strcmp(key, "width")) {
        lua_pushnumber(L, self->width);
    } else if (!::strcmp(key, "height")) {
        lua_pushnumber(L, self->height);
    } else if (!::strcmp(key, "size")) {
        lua_pushvector2(L, self->width, self->height);
    } else if (!::strcmp(key, "save")) {
        lua_pushcfunction(L, image_save);
    } else if (!::strcmp(key, "map")) {
        lua_pushcfunction(L, image_map);
    } else if (!::strcmp(key, "reduce")) {
        lua_pushcfunction(L, image_reduce);
    } else if (!::strcmp(key, "crop")) {
        lua_pushcfunction(L, image_crop);
    } else if (!::strcmp(key, "clone")) {
        lua_pushcfunction(L, image_clone);
    } else if (!::strcmp(key, "rms")) {
        lua_pushcfunction(L, image_rms);
    } else if (!::strcmp(key, "pow")) {
        lua_pushcfunction(L, image_pow);
    } else {
        my_lua_error(L, "Not a readable Image field: \""+std::string(key)+"\"");
    }
    return 1;
}

static int image_call (lua_State *L)
{
    unsigned x;
    unsigned y;
    switch (lua_gettop(L)) {
        case 2:
        check_coord(L, 2, x, y);
        break;
        case 3:
        x = check_t<unsigned>(L, 2);
        y = check_t<unsigned>(L, 3);
        break;
        default:
        my_lua_error(L, "Only allowed: image(x,y) or image(vector2(x,y))");
        return 1;
    }
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);

    if (x>=self->width || y>=self->height) {
        std::stringstream ss;
        ss << "Pixel coordinates out of range: (" << x << "," << y << ")";
        my_lua_error(L, ss.str());
    }
    switch (self->channels()) {
        case 1:
        push_pixel<1>(L, static_cast<Pixel<1>&>(self->pixel(x,y)));
        break;

        case 2:
        push_pixel<2>(L, static_cast<Pixel<2>&>(self->pixel(x,y)));
        break;

        case 3:
        push_pixel<3>(L, static_cast<Pixel<3>&>(self->pixel(x,y)));
        break;

        case 4:
        //push_pixel<4>(L, self->pixel(x,y));
        //break;

        default:
        my_lua_error(L, "Internal error: image seems to have an unusual number of channels.");
    }
    return 1;
}

static int image_add (lua_State *L)
{
    check_args(L,2);
    int a = 1, b = 2;
    if (!lua_isuserdata(L,1)) {
        std::swap(a,b);
    }
    ImageBase *self = check_ptr<ImageBase>(L, a, IMAGE_TAG);
    if (lua_isuserdata(L,b)) {
        ImageBase *other = check_ptr<ImageBase>(L, b, IMAGE_TAG);
        if (!self->compatibleWith(other)) {
            std::stringstream ss;
            ss << "Images are incompatible: " << *self << " and " << *other;
            my_lua_error(L, ss.str());
        }
        push_image(L, self->add(other));
    } else {
        switch (self->channels()) {
            case 1: {
                Pixel<1> other;
                if (!check_pixel<1>(L, other, b)) my_lua_error(L, "Cannot add this value to a 1 channel image.");
                push_image(L, self->add(other));
            }
            break;

            case 2: {
                Pixel<2> other;
                if (!check_pixel<2>(L, other, b)) my_lua_error(L, "Cannot add this value to a 2 channel image.");
                push_image(L, self->add(other));
            }
            break;

            case 3: {
                Pixel<3> other;
                if (!check_pixel<3>(L, other, b)) my_lua_error(L, "Cannot add this value to a 3 channel image.");
                push_image(L, self->add(other));
            }
            break;

            /*
            case 4: {
                Pixel<4> other;
                if (!check_pixel<4>(L, other, b)) my_lua_error(L, "Cannot add this value to a 4 channel image.");
                push_image(L, self->add(other));
            }
            break;
            */

            default:
            my_lua_error(L, "Channels must be 1, 2, 3, or 4.");
        }
    }
    return 1;
}

static int image_sub (lua_State *L)
{
    check_args(L,2);
    int a = 1, b = 2;
    bool swapped = false;
    if (!lua_isuserdata(L,a)) {
        std::swap(a,b);
        swapped = true;
    }
    ImageBase *self = check_ptr<ImageBase>(L, a, IMAGE_TAG);
    if (lua_isuserdata(L,b)) {
        ImageBase *other = check_ptr<ImageBase>(L, b, IMAGE_TAG);
        if (!self->compatibleWith(other)) {
            std::stringstream ss;
            ss << "Images are incompatible: " << *self << " and " << *other;
            my_lua_error(L, ss.str());
        }
        push_image(L, swapped ? other->sub(self) : self->sub(other));
    } else {
        switch (self->channels()) {
            case 1: {
                Pixel<1> other;
                if (!check_pixel<1>(L, other, b)) my_lua_error(L, "Cannot subtract this value from a 1 channel image.");
                push_image(L, self->sub(other, swapped));
            }
            break;

            case 2: {
                Pixel<2> other;
                if (!check_pixel<2>(L, other, b)) my_lua_error(L, "Cannot subtract this value from a 2 channel image.");
                push_image(L, self->sub(other, swapped));
            }
            break;

            case 3: {
                Pixel<3> other;
                if (!check_pixel<3>(L, other, b)) my_lua_error(L, "Cannot subtract this value from a 3 channel image.");
                push_image(L, self->sub(other, swapped));
            }
            break;

            /*
            case 4: {
                Pixel<4> other;
                if (!check_pixel<4>(L, other, b)) my_lua_error(L, "Cannot subtract this value from a 4 channel image.");
                push_image(L, self->sub(other, swapped));
            }
            break;
            */

            default:
            my_lua_error(L, "Channels must be 1, 2, 3, or 4.");
        }
    }
    return 1;
}

static int image_mul (lua_State *L)
{
    check_args(L,2);
    int a = 1, b = 2;
    if (!lua_isuserdata(L,a)) {
        std::swap(a,b);
    }
    ImageBase *self = check_ptr<ImageBase>(L, a, IMAGE_TAG);
    if (lua_isuserdata(L,b)) {
        ImageBase *other = check_ptr<ImageBase>(L, b, IMAGE_TAG);
        if (!self->compatibleWith(other)) {
            std::stringstream ss;
            ss << "Images are incompatible: " << *self << " and " << *other;
            my_lua_error(L, ss.str());
        }
        push_image(L, self->mul(other));
    } else {
        switch (self->channels()) {
            case 1: {
                Pixel<1> other;
                if (!check_pixel<1>(L, other, b)) my_lua_error(L, "Cannot multiply a 1 channel image by this value.");
                push_image(L, self->mul(other));
            }
            break;

            case 2: {
                Pixel<2> other;
                if (!check_pixel<2>(L, other, b)) my_lua_error(L, "Cannot multiply a 2 channel image by this value.");
                push_image(L, self->mul(other));
            }
            break;

            case 3: {
                Pixel<3> other;
                if (!check_pixel<3>(L, other, b)) my_lua_error(L, "Cannot multiply a 3 channel image by this value.");
                push_image(L, self->mul(other));
            }
            break;

            /*
            case 4: {
                Pixel<4> other;
                if (!check_pixel<4>(L, other, b)) my_lua_error(L, "Cannot multiply a 4 channel image by this value.");
                push_image(L, self->mul(other));
            }
            break;
            */

            default:
            my_lua_error(L, "Channels must be 1, 2, 3, or 4.");
        }
    }
    return 1;
}

static int image_div (lua_State *L)
{
    check_args(L,2);
    int a = 1, b = 2;
    bool swapped = false;
    if (!lua_isuserdata(L,1)) {
        std::swap(a,b);
        swapped = true;
    }
    ImageBase *self = check_ptr<ImageBase>(L, a, IMAGE_TAG);
    if (lua_isuserdata(L,b)) {
        ImageBase *other = check_ptr<ImageBase>(L, b, IMAGE_TAG);
        if (!self->compatibleWith(other)) {
            std::stringstream ss;
            ss << "Images are incompatible: " << *self << " and " << *other;
            my_lua_error(L, ss.str());
        }
        push_image(L, swapped ? other->div(self) : self->div(other));
    } else {
        switch (self->channels()) {
            case 1: {
                Pixel<1> other;
                if (!check_pixel<1>(L, other, b)) my_lua_error(L, "Cannot divide a 1 channel image by this value.");
                push_image(L, self->div(other, swapped));
            }
            break;

            case 2: {
                Pixel<2> other;
                if (!check_pixel<2>(L, other, b)) my_lua_error(L, "Cannot divide a 2 channel image by this value.");
                push_image(L, self->div(other, swapped));
            }
            break;

            case 3: {
                Pixel<3> other;
                if (!check_pixel<3>(L, other, b)) my_lua_error(L, "Cannot divide a 3 channel image by this value.");
                push_image(L, self->div(other, swapped));
            }
            break;

            /*
            case 4: {
                Pixel<4> other;
                if (!check_pixel<4>(L, other, b)) my_lua_error(L, "Cannot divide a 4 channel image by this value.");
                push_image(L, self->div(other, swapped));
            }
            break;
            */

            default:
            my_lua_error(L, "Channels must be 1, 2, 3, or 4.");
        }
    }
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

    {NULL, NULL}
};



template<unsigned ch> Image<ch> *image_from_lua_func (lua_State *L, unsigned width, unsigned height, int func_index)
{
    Image<ch> *my_image = new Image<ch>(width, height);
    Pixel<ch> p(0);
    for (unsigned y=0 ; y<height ; ++y) {
        for (unsigned x=0 ; x<width ; ++x) {
            lua_pushvalue(L, func_index);
            lua_pushvector2(L, x, y);
            int status = lua_pcall(L, 1, 1, 0); 
            if (status == 0) {
                if (!check_pixel<ch>(L, p, -1)) {
                    delete my_image;
                    const char *msg = lua_tostring(L, -1);
                    std::stringstream ss;
                    ss << "While initialising the image at (" << x << "," << y << "): returned value \""<<msg<<"\" has the wrong type.";
                    my_lua_error(L, ss.str());
                }
                my_image->pixel(x,y) = p;
            } else {
                const char *msg = lua_tostring(L, -1);
                delete my_image;
                std::stringstream ss;
                ss << "While initialising the image at (" << x << "," << y << "): " << msg;
                my_lua_error(L, ss.str());
            }
            lua_pop(L, 1);
        }   
    }   
    return my_image;
}

static int global_make (lua_State *L)
{
    check_args(L,3);
    // sz, channels, func
    float width_, height_;
    lua_checkvector2(L, 1, &width_, &height_);
    unsigned channels = check_int(L, 2, 1, 4);
    if (width_<0 || height_<0 || width_>65536 || height_ > 65536) {
        std::stringstream ss;
        ss << "Tried to make an image with invalid dimensions (" << width_ << "," << height_ << ")";
        my_lua_error(L, ss.str());
    }
    unsigned width = unsigned(width_);
    unsigned height = unsigned(height_);
    ImageBase *image = NULL;
    switch (lua_type(L, 3)) {
        case LUA_TNUMBER:  {
            if (channels != 1) my_lua_error(L, "If initial colour is a number, image must have 1 channel.");
            float init[1] = { (float) lua_tonumber(L, 3) };
            image = image_make(width, height, init);
        }
        break;
        case LUA_TVECTOR2: {
            if (channels != 2) my_lua_error(L, "If initial colour is a vector2, image must have 2 channels.");
            float init[2];
            lua_checkvector2(L, 3, &init[0], &init[1]);
            image = image_make(width, height, init);
        }
        break;
        case LUA_TVECTOR3: {
            if (channels != 3) my_lua_error(L, "If initial colour is a vector3, image must have 3 channels.");
            float init[3];
            lua_checkvector3(L, 3, &init[0], &init[1], &init[2]);
            image = image_make(width, height, init);
        }
        break;
        case LUA_TFUNCTION: {
            switch (channels) {
                case 1:
                image = image_from_lua_func<1>(L, width, height, 3);
                break;

                case 2:
                image = image_from_lua_func<2>(L, width, height, 3);
                break;

                case 3:
                image = image_from_lua_func<3>(L, width, height, 3);
                break;

                case 4:
                default:
                my_lua_error(L, "Channels must be either 1, 2, 3, or 4.");
            }
        }
        break;
        default:
        my_lua_error(L, "Expected a number, vector, or function at index 3.");
    }

    push_image(L, image);
    return 1;
}

static int global_open (lua_State *L)
{
    check_args(L,1);
    std::string filename = luaL_checkstring(L,1);
    ImageBase *image = image_load(filename);
    if (image == NULL) {
        lua_pushnil(L);
    } else {
        push_image(L, image);
    }
    return 1;
}

static const luaL_reg global[] = {
    {"make", global_make},
    {"open", global_open},

    {NULL, NULL}
};


void lua_wrappers_image_init (lua_State *L)
{
    luaL_newmetatable(L, IMAGE_TAG);
    luaL_register(L, NULL, image_meta_table);
    lua_pop(L,1);

    luaL_register(L, "_G", global);
    lua_pop(L, 1);
}

void lua_wrappers_image_shutdown (lua_State *L)
{
    (void) L;
}
