CC = gcc
INCLUDES -include /usr/include/errno.h
FLAGS = -DDEBUG
CPP = -lstdc++

all:
	$(CC) $(FLAGS) damosh -o -lreadline damoshparser.cpp damosh.cpp $(CPP)
