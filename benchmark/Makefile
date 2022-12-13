include /usr/local/gasnet/include/mpi-conduit/mpi-seq.mak

FLAGS = -g -I../include/shray2 -O3 -march=native
GASNET_PROGRAMS = $(wildcard gasnet/*.c)
SHRAY_PROGRAMS = $(wildcard shray/*.c)
SEQUENTIAL_PROGRAMS = $(wildcard seq/*.c)
GASNET = $(patsubst gasnet/%.c, bin/%_gasnet, $(GASNET_PROGRAMS))
SHRAY = $(patsubst shray/%.c, bin/%_shray, $(SHRAY_PROGRAMS))
SEQUENTIAL = $(patsubst seq/%.c, bin/%, $(SEQUENTIAL_PROGRAMS))

all: $(GASNET) $(SHRAY) $(SEQUENTIAL)

shray.o: ../src/shray.c ../include/shray2/shray.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

%.o: gasnet/%.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

%.o: shray/%.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/%_gasnet: %.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_shray: %.o shray.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%: seq/%.c
	gcc $(FLAGS) $^ -o $@

clean:
	$(RM) bin/*
