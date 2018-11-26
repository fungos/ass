#!/bin/bash
clang++ ../ass.cpp ../stb_vorbis.c sample.cpp -DWITH_SDL2 -I/usr/include/SDL2 -I/usr/include/AL -g -lSDL2 -lasound -lopenal -ldl -pthread -o sample
