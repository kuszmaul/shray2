include /usr/local/gasnet/include/mpi-conduit/mpi-seq.mak

SHMEM_C = /home/thomas/repos/shmemBuild/bin/oshcc
FLAGS = -pg -I../include/shray2
GASNET_PROGRAMS = $(wildcard gasnet/*.c)
SHMEM_PROGRAMS = $(wildcard shmem/*.c)
SHRAY_PROGRAMS = $(wildcard shray/*.c)
GASNET = $(patsubst gasnet/%.c, bin/%_gasnet, $(GASNET_PROGRAMS))
SHMEM = $(patsubst shmem/%.c, bin/%_shmem, $(SHMEM_PROGRAMS))
SHRAY = $(patsubst shray/%.c, bin/%_shray, $(SHRAY_PROGRAMS))

all: $(GASNET) $(SHMEM) $(SHRAY)

shray.o: ../src/shray.c ../include/shray2/shray.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

%.o: gasnet/%.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

%.o: shray/%.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/%_gasnet: %.o 
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_shmem: shmem/%.c
	$(SHMEM_C) $(FLAGS) $< -o $@

bin/%_shray: %.o shray.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

clean:
	$(RM) bin/*
