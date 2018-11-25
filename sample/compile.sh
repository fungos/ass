#!/bin/bash
clang++ ../ass.cpp ../stb_vorbis.c sample.cpp -I/usr/include/SDL2 -I/usr/include/AL -g -lSDL2 -lasound -ldl -pthread -o sample
