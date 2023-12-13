CC = gcc
INCLUDES -include /usr/include/errno.h
FLAGS = -DDEBUG
CPP = -lstdc++

all:
	$(CC) $(FLAGS) -o damosh -lreadline damoshparser.cpp damosh.cpp $(CPP)
