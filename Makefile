#include /usr/local/gasnet/include/smp-conduit/smp-seq.mak 
include /usr/local/gasnet/include/mpi-conduit/mpi-par.mak 

FORTRAN_C = gfortran
FLAGS = -O3 -march=native -mtune=native -Wall -ffast-math -Wextra -pedantic
LFLAGS = -lm -lblis64 -fsanitize=undefined -pthread
FORTRAN_FLAGS = -O3 -march=native -mtune=native -Wall -ffast-math -fcoarray=lib
FORTRAN_LFLAGS = -lcaf_openmpi
APPS = $(wildcard apps/*.c)
FORTRANAPPS = $(wildcard apps/*.f90)
RELEASE = $(patsubst apps/%.c, bin/%, $(APPS))
DEBUG = $(patsubst apps/%.c, bin/%_debug, $(APPS))
PROFILE = $(patsubst apps/%.c, bin/%_profile, $(APPS))
FORTRAN = $(patsubst apps/%.f90, bin/%_fortran, $(FORTRANAPPS))

all: release debug profile $(FORTRAN)

release: $(RELEASE)

debug: FLAGS += -DDEBUG -g -fsanitize=undefined
debug: $(DEBUG)

profile: FLAGS += -DPROFILE
profile: $(PROFILE)

bin/shray.o: src/shray.c include/shray.h include/shrayInternal.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/shray_debug.o: src/shray.c include/shray.h include/shrayInternal.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/shray_profile.o: src/shray.c include/shray.h include/shrayInternal.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

%.o: apps/%.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@ 

bin/%: %.o bin/shray.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_debug: %.o bin/shray_debug.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_profile: %.o bin/shray_profile.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/%_fortran: apps/%.f90
	$(FORTRAN_C) $(FORTRAN_FLAGS) $< -o $@ $(FORTRAN_LFLAGS)

runMulti:
	export BLIS_NUM_THREADS=2
	mpirun -n 2 bin/blas_debug 4000

clean:
	$(RM) bin/* *.mod
