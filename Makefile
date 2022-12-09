include SHRAYmake.inc
include $(GASNET_CONDUIT)

CFLAGS = -O3 -march=native -mtune=native -Wall -ffast-math -Wextra -pedantic \
		 -fno-math-errno -Iinclude
LFLAGS = -lm -pthread $(BLAS) $(GLOBALARRAYS) $(ARMCI) $(LAPACK) $(SCALAPACK) $(COARRAY_FORTRAN)
FORTRAN_FLAGS = -O3 -march=native -mtune=native -Wall -ffast-math -fcoarray=lib
CHAPEL_FLAGS = --fast -O

FORTRANAPPS = $(wildcard examples/fortran/*.f90)
FORTRAN = $(patsubst examples/fortran/%.f90, bin/fortran/%, $(FORTRANAPPS))

CHAPELAPPS = $(wildcard examples/chapel/*.chpl)
CHAPEL = $(patsubst examples/chapel/%.chpl, bin/chapel/%, $(CHAPELAPPS))

MPIAPPS = $(wildcard examples/mpi/*.c)
MPI = $(patsubst examples/mpi/%.c, bin/mpi/%, $(MPIAPPS))

UPCAPPS = $(wildcard examples/upc/*.c)
UPC = $(patsubst examples/upc/%.c, bin/upc/%, $(UPCAPPS))

GAAPPS = $(wildcard examples/globalarrays/*.c)
GA = $(patsubst examples/globalarrays/%.c, bin/globalarrays/%, $(GAAPPS))

SHRAYAPPS = $(wildcard examples/shray/*.c)
SHRAY = $(patsubst examples/shray/%.c, bin/shray/%, $(SHRAYAPPS))

all: $(SHRAY) $(MPI) $(UPC) $(FORTRAN) $(CHAPEL)
.PHONY: all clean cleanShray

release: cleanShray $(SHRAY)

debug: CFLAGS += -DDEBUG -g -fsanitize=undefined -fbounds-check -fsanitize=address
debug: LFLAGS += -g -fsanitize=undefined -fbounds-check -fsanitize=address
debug: cleanShray $(SHRAY)

profile: CFLAGS += -DPROFILE -pg
profile: LFLAGS += -pg
profile: cleanShray $(SHRAY)

bin/shray/shray.o: src/shray.c include/shray2/shray.h src/shray.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(CFLAGS) -c $< -o $@

bin/shray/queue.o: src/queue.c src/queue.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(CFLAGS) -c $< -o $@

bin/shray/%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: examples/shray/%.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(CFLAGS) -c $< -o $@

bin/shray/%: %.o bin/shray/shray.o bin/shray/bitmap.o bin/shray/queue.o bin/shray/ringbuffer.o \
	bin/csr.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/fortran/%: examples/fortran/%.f90
	$(FORTRAN_C) $(FORTRAN_FLAGS) $^ -o $@ $(LFLAGS)

bin/chapel/%: examples/chapel/%.chpl
	chpl $< $(CHAPEL_FLAGS) -o $@ $(BLAS)

bin/mpi/%: examples/mpi/%.c
	$(MPICC) $< $(CFLAGS) -o $@ $(LFLAGS)

bin/upc/%: examples/upc/%.c bin/csr.o
	$(UPCC) -g $^ -o $@ $(BLAS)

bin/csr.o: examples/util/csr.c examples/util/csr.h
	$(CC) $(CFLAGS) -c $< -o $@

bin/globalarrays/%: examples/globalarrays/%.c bin/csr.o
	$(MPICC) $(CFLAGS) $^ -o $@ $(LFLAGS) $(FORTRANLIB)

clean:
	$(RM) bin/shray/* bin/chapel/* bin/fortran/* bin/mpi/* bin/upc/* *.mod bin/csr.o

cleanShray:
	$(RM) bin/shray/*
