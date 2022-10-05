CC = /home/thomas/repos/shmemBuild/bin/oshcc
FLAGS = -O3 -march=native -mtune=native -Wall -ffast-math -std=gnu11 -g
LFLAGS = -lm -lblis64 -fsanitize=undefined -pthread
APPS = $(wildcard apps/*.c)
RELEASE = $(patsubst apps/%.c, bin/%, $(APPS))

all: release

release: $(RELEASE)

%.o: apps/%.c
	$(CC) $(CFLAGS) $(FLAGS) -c $< -o $@

bin/%: %.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LFLAGS)

clean:
	$(RM) bin/*
