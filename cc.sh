#!/bin/sh

gcc -std=c99 -Wall -pedantic -g -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -lm -lpthread -lircclient src/distributions.c src/irc.c src/kalemattu.c -o kalemattuc
