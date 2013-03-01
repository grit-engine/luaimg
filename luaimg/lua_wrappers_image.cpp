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
    if (!lua_type(L, index) != LUA_TFUNCTION) {
        std::stringstream msg;
        msg << "Expected a function at index " << index;
        my_lua_error(L,msg.str());
    }
}

template<class T> T* check_ptr (lua_State *L, int index, const char *tag)
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




void lua_push_image (lua_State *L, ImageBase *image)
{
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
    ss << "Image ("<<self->width<<","<<self->height<<")@"<<self->channels()<<" [0x"<<self<<"]";;
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
    } else {
        my_lua_error(L, "Not a readable Image field: \""+std::string(key)+"\"");
    }
    return 1;
}

static int image_call (lua_State *L)
{
    float x_, y_;
    unsigned x;
    unsigned y;
    switch (lua_gettop(L)) {
        case 2:
        lua_checkvector2(L, 2, &x_, &y_);
        x = x_ < 0 ? 0 : x_ > 65535 ? 65535 : x_;
        y = y_ < 0 ? 0 : y_ > 65535 ? 65535 : y_;
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

    if (x>self->width || y>self->height) {
        std::stringstream ss;
        ss << "Pixel coordinates out of range: (" << x << "," << y << ")";
        my_lua_error(L, ss.str());
    }
    switch (self->channels()) {
        case 1:
        lua_pushnumber(L, static_cast<Image<1>*>(self)->pixel(x,y)[0]);
        break;

        case 2:
        lua_pushvector2(L, static_cast<Image<2>*>(self)->pixel(x,y)[0],
                           static_cast<Image<2>*>(self)->pixel(x,y)[1]);
        break;

        case 3:
        lua_pushvector3(L, static_cast<Image<3>*>(self)->pixel(x,y)[0],
                           static_cast<Image<3>*>(self)->pixel(x,y)[1],
                           static_cast<Image<3>*>(self)->pixel(x,y)[2]);
        break;

        case 4:

        default:
        my_lua_error(L, "Internal error: image seems to have an unusual number of channels.");
    }
    return 1;
}


const luaL_reg image_meta_table[] = {
    {"__tostring", image_tostring},
    {"__gc",       image_gc},
    {"__index",    image_index},
    {"__eq",       image_eq},
    {"__call",     image_call},
    //{"__mul",      image_mul},
    //{"__unm",      image_unm}, 
    //{"__add",      image_add}, 
    //{"__sub",      image_sub}, 
    //{"__div",      image_div}, 

    {NULL, NULL}
};


static int global_make (lua_State *L)
{
    check_args(L,3);
    // sz, channels, func
    float width, height;
    lua_checkvector2(L, 1, &width, &height);
    unsigned channels = luaL_checknumber(L, 2);
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
                my_lua_error(L, "Channels must be either 1, 2, 3, or 4.");

                case 2:
                my_lua_error(L, "Channels must be either 1, 2, 3, or 4.");

                case 3: {
                    Image<3> *my_image = new Image<3>(width, height);
                    for (unsigned y=0 ; y<my_image->height ; ++y) {
                        for (unsigned x=0 ; x<my_image->width ; ++x) {
                            lua_pushvalue(L, 3);
                            lua_pushvector2(L, x, y);
                            int status = lua_pcall(L, 1, 1, 0); 
                            if (status == 0) {
                                if (lua_type(L,-1) == LUA_TVECTOR3) {
                                    float r,g,b;
                                    lua_checkvector3(L, -1, &r, &g, &b);
                                    my_image->pixel(x, y)[0] = r;
                                    my_image->pixel(x, y)[1] = g;
                                    my_image->pixel(x, y)[2] = b;
                                } else {
                                    delete my_image;
                                    std::stringstream ss;
                                    ss << "While initialising the image at (" << x << "," << y << "): returned value was not a vector3.";
                                    my_lua_error(L, ss.str());
                                }
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
                    image = my_image;
                }
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

    lua_push_image(L, image);
    return 1;
}

static int global_open (lua_State *L)
{
    check_args(L,1);
    std::string filename = luaL_checkstring(L,1);
    ImageBase *image = image_load(filename);
    lua_push_image(L, image);
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
