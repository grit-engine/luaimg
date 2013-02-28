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
#include <memory>

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




void lua_push_image (lua_State *L, ImageBase *image)
{
    (void) image;
    lua_pushnil(L);
}


template<class T> T* check_ptr (lua_State *L, int index, const char *tag)
{
    return static_cast<T*>(luaL_checkudata(L, index, tag));
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

static int image_index (lua_State *L)
{
    check_args(L,2);
    ImageBase *self = check_ptr<ImageBase>(L, 1, IMAGE_TAG);
    const char *key = luaL_checkstring(L, 2);
    (void) self;
    lua_pushstring(L, key);
    return 1;
}


const luaL_reg image_meta_table[] = {
    {"__tostring", image_tostring},
    {"__gc",       image_gc},
    {"__index",    image_index},
    {"__eq",       image_eq},
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
            lua_checkvector3(L, 3, &init[0], &init[1], &init[3]);
            image = image_make(width, height, init);
        }
        break;
        case LUA_TFUNCTION: {
            switch (channels) {
                case 1:
                case 2:
                case 3:
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

static const luaL_reg global[] = {
    {"make", global_make},

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
