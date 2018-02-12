#!/bin/bash

TAR = xnode
CFLAGS = -Wall -g
LIB = -lpthread

SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOURCES))

all:$(TAR)
xnode : $(OBJS)
	cc -o $@ $^ $(CFLAGS) $(LIB)
	
	
.PHNOY:clean
clean:
	-rm -rf *.o $(TAR)
