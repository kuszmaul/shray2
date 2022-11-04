include /usr/local/gasnet/include/mpi-conduit/mpi-seq.mak

SHMEM_C = /home/thomas/repos/shmemBuild/bin/oshcc
FLAGS = -pg
GASNET_PROGRAMS = $(wildcard gasnet/*.c)
SHMEM_PROGRAMS = $(wildcard shmem/*.c)
GASNET = $(patsubst gasnet/%.c, bin/%_gasnet, $(GASNET_PROGRAMS))
SHMEM = $(patsubst shmem/%.c, bin/%_shmem, $(SHMEM_PROGRAMS))

all: $(GASNET) $(SHMEM)

%.o: gasnet/%.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/%_gasnet: %.o 
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_shmem: shmem/%.c
	$(SHMEM_C) $(FLAGS) $< -o $@

clean:
	$(RM) bin/*
