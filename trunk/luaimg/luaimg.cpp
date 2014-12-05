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

#include <cstdlib>

#include <string>
#include <vector>
#include <sstream>
#include <iostream>


extern "C" {
	#include <FreeImage.h>
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}


#include "interpreter.h"
#include "Image.h"
#include "text.h"

#define LUAIMG_VERSION "0.9"

const char *prompt = "luaimg> ";

const char *info =
    "luaimg (c) Dave Cunningham 2013  (version: " LUAIMG_VERSION ")\n"
    "A command-line image processing tool based on the Lua language.\n"
;

const char *usage =
    "Usage: luaimg { <opt> | <arg> }\n\n"
    "where <opt> ::= -h | --help                     This message\n"
    "              | -e <snippet> | --exec <snippet> Execute the given Lua snippet\n"
    "              | -f <file> | --file <file>       Execute the given file containing Lua code\n"
    "              | --                              End of commandline options\n"
    "              | -F <file> | --File <file>       Short-hand for -f <file> --\n"
    "              | -i | --interactive              Enter interactive mode after processing -e and -f\n"
    "              | -p <str> | --prompt <str>       Override the interactive prompt (default \"luaimg> \")\n"
    "Scripts and snippets are executed in sequence.\n"
    "The non-option <arg> list is passed to the code via the Lua ... construct.\n"
;


void my_freeimage_error (FREE_IMAGE_FORMAT fif, const char *str)
{
    (void) fif;
    std::cerr << str << std::endl;
}


std::string next_arg(int& so_far, int argc, char **argv)
{
        if (so_far==argc) {
                std::cerr<<"ERROR: Not enough commandline arguments...\n"<<std::endl;
                std::cerr<<usage<<std::endl;
                exit(EXIT_FAILURE);
        }
        return argv[so_far++];
}

enum FileOrSnippet { F, S };

int main (int argc, char **argv)
{

    bool interactive = false;
    typedef std::vector<std::pair<FileOrSnippet,std::string>> Work;
    Work work;
    int so_far = 1; 
    bool no_more_switches = false;
    std::vector<std::string> args;
    std::string prompt = "luaimg> ";
    while (so_far < argc) {
        std::string arg = next_arg(so_far, argc, argv);
        if (no_more_switches) {
            args.push_back(arg);
        } else if (arg=="-h" || arg=="--help") {
            std::cout<<info<<std::endl;
            std::cout<<usage<<std::endl;
            exit(EXIT_SUCCESS);
        } else if (arg=="--") {
            no_more_switches = true;
        } else if (arg=="-i" || arg=="--interactive") {
            interactive = true;
        } else if (arg=="-p" || arg=="--prompt") {
            prompt = next_arg(so_far,argc,argv);
        } else if (arg=="-f" || arg=="--file") {
            work.push_back(std::pair<FileOrSnippet,std::string>(F, next_arg(so_far,argc,argv)));
        } else if (arg=="-F" || arg=="--File") {
            work.push_back(std::pair<FileOrSnippet,std::string>(F, next_arg(so_far,argc,argv)));
            no_more_switches = true;
        } else if (arg=="-e" || arg=="--exec") {
            work.push_back(std::pair<FileOrSnippet,std::string>(S, next_arg(so_far,argc,argv)));
        } else {
            args.push_back(arg);
        }
    }

    if (work.size()==0 && !interactive) {
        std::cerr<<info;

        std::cerr<<
                "Entering the LuaImg shell.  Use commandline option -i to suppress this message\n"
                "in future.  Check other options with --help. Want to quit?  Use ctrl+d or\n"
                "ctrl+c.  At the following prompt, give the LuaImg statements you want to\n"
                "execute.  \"Make good art.\" -- Neil Gaiman" << std::endl;

        interactive = true;
    }


    FreeImage_Initialise();
    FreeImage_SetOutputMessage(my_freeimage_error);

    text_init();

    interpreter_init();

    unsigned snippet_counter = 0;
    for (Work::iterator i=work.begin(),i_=work.end() ; i!=i_ ; ++i) {
        switch (i->first) {
            case F:
            if (!interpreter_exec_file(i->second, args)) {
                return EXIT_FAILURE;
            }
            break;

            case S: {
                snippet_counter++;
                std::stringstream ss;
                ss << "[snippet" << snippet_counter << "]";
                if (!interpreter_exec_snippet(i->second, args, ss.str())) {
                    return EXIT_FAILURE;
                }
            }
            break;
        }
    }

    if (interactive) {
        interpreter_exec_interactively(prompt);
    } 

    interpreter_shutdown();

    FreeImage_DeInitialise();

    return EXIT_SUCCESS;
}

