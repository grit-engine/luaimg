#!/bin/bash

make -C .. && ../luaimg -F doc.lua true true true && scp {api,download,examples,index,usage}.html *.gif *.png *.css gritengine@dog.woaf.net:public_html/luaimg && ../luaimg -F doc.lua false false false
