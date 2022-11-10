CC = gcc
FLAGS = -Wall -Wextra -O3 -march=native

all: bin/bitmap

bin/bitmap: bitmap.c ../src/utils.c
	$(CC) $(FLAGS) $< -o $@

clean:
	$(RM) bin/*
