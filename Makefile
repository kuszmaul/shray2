#include /usr/local/gasnet/include/smp-conduit/smp-seq.mak
include /usr/local/gasnet/include/mpi-conduit/mpi-seq.mak
#include /usr/local/gasnet/include/udp-conduit/udp-seq.mak

FORTRAN_C = gfortran
CHAPEL_C = chpl
SHMEM_C = oshcc
FLAGS = -O3 -march=native -mtune=native -Wall -ffast-math -Wextra -pedantic -fno-math-errno -Iinclude -pg
LFLAGS = -lm -lblis64 -fsanitize=undefined -pthread -pg
#LFLAGS = -lm -lopenblas -fsanitize=undefined -pthread
FORTRAN_FLAGS = -O3 -march=native -mtune=native -Wall -ffast-math -fcoarray=lib -g
FORTRAN_LFLAGS = -lcaf_openmpi
CHAPEL_FLAGS = --fast
CHAPEL_LFLAGS = -lblis64
APPS = $(wildcard examples/*.c)
FORTRANAPPS = $(wildcard examples/fortran/*.f90)
SHMEMAPPS = $(wildcard examples/oshmem/*.c)
CHAPELAPPS = $(wildcard examples/chapel/*.chpl)
RELEASE = $(patsubst examples/%.c, bin/shray/%, $(APPS))
DEBUG = $(patsubst examples/%.c, bin/shray/%_debug, $(APPS))
PROFILE = $(patsubst examples/%.c, bin/shray/%_profile, $(APPS))
GRAPH = $(patsubst examples/%.c, bin/%_graph, $(APPS))
FORTRAN = $(patsubst examples/fortran/%.f90, bin/fortran/%_fortran, $(FORTRANAPPS))
SHMEM = $(patsubst examples/oshmem/%.c, bin/oshmem/%_oshmem, $(SHMEMAPPS))
CHAPEL = $(patsubst examples/chapel/%.chpl, bin/chapel/%, $(CHAPELAPPS))

all: release debug profile $(FORTRAN) $(CHAPEL)

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

bin/queue.o: src/queue.c src/queue.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

%.o: examples/%.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/shray/%: %.o bin/shray.o bin/bitmap.o bin/queue.o bin/ringbuffer.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/shray/%_debug: %.o bin/shray_debug.o bin/bitmap.o bin/queue.o bin/ringbuffer.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/shray/%_profile: %.o bin/shray_profile.o bin/bitmap.o bin/queue.o bin/ringbuffer.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/fortran/%_fortran: examples/fortran/%.f90
	$(FORTRAN_C) $(FORTRAN_FLAGS) $< -o $@ $(FORTRAN_LFLAGS)

bin/oshmem/%_oshmem: examples/oshmem/%.c
	$(SHMEM_C) $(FLAGS) $< -o $@ $(LFLAGS)

bin/chapel/%: examples/chapel/%.chpl
	chpl $< $(CHAPEL_FLAGS) -o $@ $(CHAPEL_LFLAGS)

testMatrix:
	export SHRAY_CACHESIZE=4096000
	export SHRAY_CACHELINE=1
	mpirun.mpich -n 4 bin/matrix_debug 1000 2>&1 | grep "\[node 1" > matrix.out

clean:
	$(RM) bin/shray/* bin/oshmem/* bin/chapel/* bin/fortran/* *.mod
