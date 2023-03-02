CC = gcc
FLAGS = -Wall -Wextra -O3 -march=native -g

all: bin/bitmap bin/intrinsics

bin/bitmap: bitmap.c ../src/bitmap.c
	$(CC) $(FLAGS) $< -o $@

bin/intrinsics: intrinsics.c 
	$(CC) $(FLAGS) $< -o $@

clean:
	$(RM) bin/*
