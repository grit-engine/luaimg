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

#include <cstdlib>

#include <string>
#include <vector>
#include <iostream>

#include <signal.h>

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

#include "interpreter.h"

static lua_State *L;

// call debug.traceback with given error message and param of 2 
static int my_lua_error_handler (lua_State *L)
{
    // STACK: [errmsg]
    lua_getfield(L, LUA_GLOBALSINDEX, "debug");
    // STACK: [errmsg, debug]
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        // STACK: [errmsg]
        return 1;
    }
    lua_getfield(L, -1, "traceback");
    // STACK: [errmsg, traceback]
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        // STACK: [errmsg]
        return 1;
    }
    lua_pushvalue(L, 1);
    // STACK: [errmsg, traceback, errmsg]
    lua_pushinteger(L, 2);
    // STACK: [errmsg, traceback, errmsg, 2]
    lua_call(L, 2, 1);
    // STACK: [r]
    return 1;
}


static bool interrupted = false;
void interpreter_interrupt_probe (void)
{
    interrupted = false;
    lua_sethook(L, NULL, 0, 0);
    luaL_error(L, "interrupted!");
}
static void reset_hook_and_interrupt (lua_State *, lua_Debug *) { interpreter_interrupt_probe(); }
static void interrupt_handler (int sig)
{
    // set default signal handler for this isgnal
    // (another interrupt signal therefore terminates process)
    signal(sig, SIG_DFL);

    std::cerr << "Attempting clean termination, (interrupt again for forced termination)." << std::endl;

    // interrupt long-running native code (ie code that will be periodically calling interrupt_probe)
    interrupted = true;

    // set a hook on any Lua interpreter event (i.e. everything except native code)
    // (the hook calls interrupt_probe)
    lua_sethook(L, reset_hook_and_interrupt, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static std::vector<std::string> no_args;

// Return codes;
// 0 (success)
// LUA_ERRRUN (runtime error -- an error message and stack has been printed)
static int execute_code (const std::vector<std::string> &args=no_args)
{
    // STACK: [func]
    int base = lua_gettop(L);
    for (unsigned i=0 ; i<args.size() ; ++i) lua_pushstring(L,args[i].c_str());
    // STACK: [func, args...]
    lua_pushcfunction(L, my_lua_error_handler);
    // STACK: [func, args..., err_handler]
    lua_insert(L, base);

    // STACK: [err_handler, func, args...]
    signal(SIGINT, interrupt_handler);
    int status = lua_pcall(L, args.size(), LUA_MULTRET, base);
    signal(SIGINT, SIG_DFL);

    if (status == LUA_ERRERR) {
        std::cerr << "ERROR: A script error occured, additionally the Lua error handler could not be executed." << std::endl;
        exit(EXIT_FAILURE);
    } else if (status == LUA_ERRMEM) {
        std::cerr << "ERROR: Out of memory while executing Lua code." << std::endl;
        exit(EXIT_FAILURE);
    }

    // STACK: [err_handler, returnvals...]

    lua_remove(L, base);

    // STACK: [returnvals...]

    return status;
}




bool interpreter_exec_file (const std::string &fname, const std::vector<std::string> &args)
{
    int status = luaL_loadfile(L, fname.c_str());

    if (status == 0) {
        status = execute_code(args);
        lua_settop(L, 0);
        return status == 0;
    } else if (status == LUA_ERRFILE) {
        const char *msg = lua_tostring(L, -1);
        std::cerr << "Could not load Lua file: \"" << fname << "\"" << std::endl;
        std::cerr << msg << std::endl;
        lua_pop(L, 1);      
        return false;
    } else if (status == LUA_ERRSYNTAX) {
        const char *msg = lua_tostring(L, -1);
        std::cerr << "Syntax error while loading Lua file: \"" << fname << "\"" << std::endl;
        std::cerr << msg << std::endl;
        lua_pop(L, 1);      
        return false;
    } else if (status == LUA_ERRMEM) {
        std::cerr << "ERROR: Out of memory while loading Lua file: \"" << fname << "\"" << std::endl;
        exit(EXIT_FAILURE);
    }

    return true;
}

// similar to luaL_loadbuffer but attempts to wrap with a return statement
int parse_with_return (const std::string &input, const std::string &name)
{
    // STACK: []
    std::string input_with_return = "return " + input;
    int status = luaL_loadbuffer(L, input_with_return.c_str(), input_with_return.length(), name.c_str());
    if (status == LUA_ERRSYNTAX) {
        // STACK: [msg]
        // try without 'return'
        lua_pop(L, 1); // ignore error message
        // STACK: []
        status = luaL_loadbuffer(L, input.c_str(), input.length(), name.c_str());
    }

    // STACK: [func]

    return status;
}

// calls 'print' using the given stack as arguments, but only if it contains something
void print_stack (void)
{
    // STACK: [args...]
    if (lua_gettop(L) == 0) {
        // STACK: []
        return;
    }

    // STACK: [args...]
    lua_getglobal(L, "print");
    // STACK: [args..., print]
    lua_insert(L, 1); 
    // STACK: [print, args...]
    int status = lua_pcall(L, lua_gettop(L)-1, 0, 0);
    if (status != 0) {
        // STACK: [msg]
        const char *msg = lua_tostring(L, -1);
        std::cerr << "ERROR: could not call \"print\" (" << msg << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    // STACK: []
}

bool interpreter_exec_snippet (const std::string &str, const std::vector<std::string> &args, const std::string &name)
{
    int status = parse_with_return(str, name);

    if (status == 0) {
        status = execute_code(args);
        if (status != 0) return false;
        print_stack();
        return true;
    } else if (status == LUA_ERRSYNTAX) {
        const char *msg = lua_tostring(L, -1);
        std::cerr << "Syntax error while parsing Lua code: \"" << str << "\"" << std::endl;
        std::cerr << msg << std::endl;
        lua_pop(L, 1);      
        return false;
    } else if (status == LUA_ERRMEM) {
        std::cerr << "ERROR: Out of memory while parsing Lua code: \"" << str << "\"" << std::endl;
        exit(EXIT_FAILURE);
    }

    return true;
}


void interpreter_exec_interactively ()
{
    // STACK: []

    while (true) {

        // STACK: []

        std::string input;

        {
            char buffer[LUA_MAXINPUT];
            char *b = buffer;

            if (lua_readline(L, b, "luaimg> ") == 0) {
                // end of stdin, exit cleanly
                break;
            }
            input = b;
        }

        // trim newline if it has one
        if (input.back() == '\n') input.erase(input.length()-1);

        if (input == "") continue;

        ///////////
        // PARSE //
        ///////////

        // first try with 'return'

        int status = parse_with_return(input, "[interactive]");

        if (status == 0) {

            /////////////
            // EXECUTE //
            /////////////

            // STACK: [func]
            status = execute_code();
            if (status == 0) {

                // STACK: [retvals...]

                print_stack();

                // STACK: []

            } else {

                // STACK: [err]
                lua_pop(L, 1);

                // STACK: []
            }
        } else if (status == LUA_ERRSYNTAX) {
            // STACK: [err]
            const char *msg = lua_tostring(L, -1);
            std::cerr << "Syntax error: " << msg << std::endl;
            lua_pop(L, 1);
            // STACK: []
        } else if (status == LUA_ERRMEM) {
            std::cerr << "ERROR: Out of memory while parsing interactive input." << std::endl;
            exit(EXIT_FAILURE);
        }

        // STACK: []
    }

    // end the 'prompt' line
    std::cout << std::endl;
}

void interpreter_init (void)
{
    L = lua_open();
    if (L == NULL) {
        std::cerr << "Internal error: could not create Lua state." << std::endl;
        exit(EXIT_FAILURE);
    }       
                
    luaL_openlibs(L);
}

void interpreter_shutdown (void)
{
    lua_close(L);
    L = NULL;
}
