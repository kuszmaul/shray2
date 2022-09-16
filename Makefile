CC = mpicc.openmpi
RUN = mpirun.openmpi
FLAGS = -O3 -ffast-math -march=native -mtune=native -Wall
LFLAGS = -lm
APPS = $(wildcard apps/*.c)
RELEASE = $(patsubst apps/%.c, bin/%, $(APPS))
DEBUG = $(patsubst apps/%.c, bin/%_debug, $(APPS))
PROFILE = $(patsubst apps/%.c, bin/%_profile, $(APPS))

all: release debug profile

release: $(RELEASE)

debug: FLAGS += -DDEBUG -g
debug: $(DEBUG)

profile: FLAGS += -DPROFILE
profile: $(PROFILE)

bin/shray.o: src/shray.c include/shray.h include/shrayInternal.h
	$(CC) $(FLAGS) -c $< -o $@

bin/shray_debug.o: src/shray.c include/shray.h include/shrayInternal.h
	$(CC) $(FLAGS) -c $< -o $@

bin/shray_profile.o: src/shray.c include/shray.h include/shrayInternal.h
	$(CC) $(FLAGS) -c $< -o $@

bin/%: apps/%.c bin/shray.o
	$(CC) $(FLAGS) $^ -o $@ $(LFLAGS)

bin/%_debug: apps/%.c bin/shray_debug.o
	$(CC) $(FLAGS) $^ -o $@ $(LFLAGS)

bin/%_profile: apps/%.c bin/shray_profile.o
	$(CC) $(FLAGS) $^ -o $@ $(LFLAGS)

debug%:
	$(RUN) -n 2 bin/$*_debug 2>&1 | vi -

clean:
	$(RM) bin/*
