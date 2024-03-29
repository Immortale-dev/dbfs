.PHONY: all generate_o generate_t dist

CC=g++
CFLAGS=-c -Wall -x c++ -std=c++17
LDFLAGS=
SRCPATH:=src/
SRCS:=$(wildcard $(SRCPATH)*.cpp)
OBJS:=$(SRCS:%.cpp=%.o)
INCL=-Isrc -Itest


all: generate_o generate_t

	
generate_o: ${OBJS}

generate_t: 
	${CC} ${INCL} -std=c++17 -o test.exe test/test.cpp ${OBJS} -pthread
	
dist: generate_o

%.o: %.cpp
	${CC} ${CFLAGS} ${INCL} $< -o $@
	
	
