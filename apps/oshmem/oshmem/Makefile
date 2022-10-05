CC = oshcc
FLAGS = -O3 -march=native -mtune=native -Wall -ffast-math -std=gnu11
LFLAGS = -lm -lopenblas -fsanitize=undefined -pthread
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
