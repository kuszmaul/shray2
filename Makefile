#include /usr/local/gasnet/include/smp-conduit/smp-seq.mak
include /usr/local/gasnet/include/mpi-conduit/mpi-seq.mak
#include /usr/local/gasnet/include/udp-conduit/udp-seq.mak

FORTRAN_C = gfortran
SHMEM_C = /home/thomas/repos/shmemBuild/bin/oshcc
FLAGS = -O3 -march=native -mtune=native -Wall -ffast-math -Wextra -pedantic -fno-math-errno -Iinclude -g
LFLAGS = -lm -lblis64 -fsanitize=undefined -pthread
#LFLAGS = -lm -lopenblas -fsanitize=undefined -pthread
FORTRAN_FLAGS = -O3 -march=native -mtune=native -Wall -ffast-math -fcoarray=lib
FORTRAN_LFLAGS = -lcaf_openmpi
APPS = $(wildcard examples/*.c)
FORTRANAPPS = $(wildcard examples/fortran/*.f90)
SHMEMAPPS = $(wildcard examples/oshmem/*.c)
RELEASE = $(patsubst examples/%.c, bin/%, $(APPS))
DEBUG = $(patsubst examples/%.c, bin/%_debug, $(APPS))
PROFILE = $(patsubst examples/%.c, bin/%_profile, $(APPS))
GRAPH = $(patsubst examples/%.c, bin/%_graph, $(APPS))
FORTRAN = $(patsubst examples/fortran/%.f90, bin/%_fortran, $(FORTRANAPPS))
SHMEM = $(patsubst examples/oshmem/%.c, bin/%_shmem, $(SHMEMAPPS))

all: release debug profile $(FORTRAN) $(SHMEM)

release: $(RELEASE)

debug: FLAGS += -DDEBUG -g -fsanitize=undefined
debug: $(DEBUG)

profile: FLAGS += -DPROFILE -pg
profile: $(PROFILE)

bin/shray.o: src/shray.c include/shray2/shray.h src/shray.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/shray_debug.o: src/shray.c include/shray2/shray.h src/shray.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/shray_profile.o: src/shray.c include/shray2/shray.h src/shray.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

#FIXME This compiler should have the same ABI as the compiler used by GASNET_CC (e.g. gcc and clang)
bin/%.o: src/%.c src/%.h
	gcc $(FLAGS) -c $< -o $@

%.o: examples/%.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/%: %.o bin/shray.o bin/bitmap.o bin/queue.o bin/ringbuffer.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_debug: %.o bin/shray_debug.o bin/bitmap.o bin/queue.o bin/ringbuffer.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_profile: %.o bin/shray_profile.o bin/bitmap.o bin/queue.o bin/ringbuffer.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_fortran: examples/fortran/%.f90
	$(FORTRAN_C) $(FORTRAN_FLAGS) $< -o $@ $(FORTRAN_LFLAGS)

bin/%_shmem: examples/oshmem/%.c
	$(SHMEM_C) $(FLAGS) $< -o $@ $(LFLAGS)

testMatrix:
	export SHRAY_CACHESIZE=4096000
	export SHRAY_CACHELINE=1
	mpirun.mpich -n 4 bin/matrix_debug 1000 2>&1 | grep "\[node 1" > matrix.out

clean:
	$(RM) bin/* *.mod
