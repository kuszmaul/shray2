CC = gcc
FLAGS = -O3 -g

all: parser

parser: mm_to_csr.c
	$(CC) $(FLAGS) $^ -o $@

clean:
	$(RM) parser
