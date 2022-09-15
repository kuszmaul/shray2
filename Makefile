CC = mpicc.openmpi
FLAGS = -O3 -g -Wall -ffast-math -march=native -mtune=native
LFLAGS = -lm
SRC = src
INC = include
BIN = bin
APP = apps
APPS = $(wildcard $(APP)/*.c)
BINS = $(patsubst $(APP)/%.c, $(BIN)/%, $(APPS))

all: $(BINS)

%.o: $(SRC)/%.c $(INC)/%.h $(INC)/utils.h
	$(CC) $(FLAGS) -c $< -o $@

$(BIN)/%: $(APP)/%.c shray.o
	$(CC) $(FLAGS) $^ -o $@ $(LFLAGS)

clean:
	$(RM) $(BIN)/*
