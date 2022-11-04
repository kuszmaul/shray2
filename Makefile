#include /usr/local/gasnet/include/smp-conduit/smp-seq.mak
include /usr/local/gasnet/include/mpi-conduit/mpi-seq.mak

FORTRAN_C = gfortran
SHMEM_C = /home/thomas/repos/shmemBuild/bin/oshcc
FLAGS = -march=native -mtune=native -Wall -ffast-math -Wextra -pedantic -fno-math-errno -Iinclude
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

graph: FLAGS += -DGRAPH
graph: $(GRAPH)

bin/shray.o: src/shray.c include/shray2/shray.h include/shray2/shrayInternal.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/shray_debug.o: src/shray.c include/shray2/shray.h include/shray2/shrayInternal.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/shray_profile.o: src/shray.c include/shray2/shray.h include/shray2/shrayInternal.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/shray_graph.o: src/shray.c include/shray2/shray.h include/shray2/shrayInternal.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

%.o: examples/%.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/%: %.o bin/shray.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_debug: %.o bin/shray_debug.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_profile: %.o bin/shray_profile.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_graph: %.o bin/shray_graph.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_fortran: examples/fortran/%.f90
	$(FORTRAN_C) $(FORTRAN_FLAGS) $< -o $@ $(FORTRAN_LFLAGS)

bin/%_shmem: examples/oshmem/%.c
	$(SHMEM_C) $(FLAGS) $< -o $@ $(LFLAGS)

runMulti:
	export BLIS_NUM_THREADS=2
	mpirun -n 2 bin/blas_debug 4000

# 2 GB per array, so 4GB total
runHuge:
	time mpirun -n 4 bin/2dstencil_profile 15811 100

clean:
	$(RM) bin/* *.mod
