.PHONY: all generate_o generate_t

CC=g++
CFLAGS=-c -Wall -x c++ -std=c++17
LDFLAGS=
SRCPATH:=src/
SRCS:=$(wildcard $(SRCPATH)*.hpp)
OBJS:=$(SRCS:%.hpp=%.o)
INCL=-Isrc -Itest


all: generate_t

	
generate_o: ${OBJS}

generate_t: 
	${CC} ${INCL} -o test.exe test/test.cpp 

%.o: %.hpp
	${CC} ${CFLAGS} ${INCL} $< -o $@
	
	
