include /usr/local/gasnet/include/mpi-conduit/mpi-seq.mak

FORTRAN_C = gfortran
MPICC = mpicc
CHAPEL_C = chpl
FLAGS = -O3 -march=native -mtune=native -Wall -ffast-math -Wextra -pedantic -fno-math-errno -Iinclude
LFLAGS = -lm -lblis64 -fsanitize=undefined -pthread -pg
FORTRAN_FLAGS = -O3 -march=native -mtune=native -Wall -ffast-math -fcoarray=lib -g
FORTRAN_LFLAGS = -lcaf_openmpi
CHAPEL_FLAGS = --fast -g
CHAPEL_LFLAGS = -lblis64

FORTRANAPPS = $(wildcard examples/fortran/*.f90)
FORTRAN = $(patsubst examples/fortran/%.f90, bin/fortran/%_fortran, $(FORTRANAPPS))

CHAPELAPPS = $(wildcard examples/chapel/*.chpl)
CHAPEL = $(patsubst examples/chapel/%.chpl, bin/chapel/%, $(CHAPELAPPS))

MPIAPPS = $(wildcard examples/mpi/*.c)
MPI = $(patsubst examples/mpi/%.c, bin/mpi/%, $(MPIAPPS))

UPCAPPS = $(wildcard examples/upc/*.c)
UPC = $(patsubst examples/upc/%.c, bin/upc/%, $(UPCAPPS))

APPS = $(wildcard examples/shray/*.c)
RELEASE = $(patsubst examples/shray/%.c, bin/shray/%, $(APPS))
DEBUG = $(patsubst examples/shray/%.c, bin/shray/%_debug, $(APPS))
PROFILE = $(patsubst examples/shray/%.c, bin/shray/%_profile, $(APPS))

all: release debug profile $(FORTRAN) $(MPI) $(CHAPEL) $(UPC)

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

bin/%.o: src/%.c src/%.h
	gcc $(FLAGS) -c $< -o $@

bin/queue.o: src/queue.c src/queue.h
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

%.o: examples/shray/%.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/shray/%: %.o bin/shray.o bin/bitmap.o bin/queue.o bin/ringbuffer.o bin/csr.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/shray/%_debug: %.o bin/shray_debug.o bin/bitmap.o bin/queue.o bin/ringbuffer.o bin/csr.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/shray/%_profile: %.o bin/shray_profile.o bin/bitmap.o bin/queue.o bin/ringbuffer.o bin/csr.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

bin/fortran/%_fortran: examples/fortran/%.f90
	$(FORTRAN_C) $(FORTRAN_FLAGS) $^ -o $@ $(FORTRAN_LFLAGS)

bin/chapel/%: examples/chapel/%.chpl
	chpl $< $(CHAPEL_FLAGS) -o $@ $(CHAPEL_LFLAGS)

bin/mpi/%: examples/mpi/%.c
	$(MPICC) $< $(FLAGS) -o $@ $(LFLAGS)

bin/upc/%: examples/upc/%.c bin/csr.o
	upcc -g $^ -o $@

bin/csr.o: examples/util/csr.c examples/util/csr.h
	gcc $(FLAGS) -c $< -o $@

clean:
	$(RM) bin/shray/* bin/chapel/* bin/fortran/* bin/mpi/* bin/upc/* *.mod
