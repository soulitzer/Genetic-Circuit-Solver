CC=gcc
CFLAGS=-g -Wall -std=c99
LDFLAGS=-g
LDLIBS=-lm

genetic: genetic.o circ.o data.o

circ.o: circ.c circ.h

data.o: data.c data.h
