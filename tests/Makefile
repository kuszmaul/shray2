CC = gcc
FLAGS = -Wall -Wextra -O3 -march=native

all: bitmap

bin/bitmap: bitmap.c 
	$(CC) $(FLAGS) $^ -o $@

clean:
	$(RM) bin/*
