include /usr/local/gasnet/include/smp-conduit/smp-seq.mak 

FLAGS = -O3 -ffast-math -march=native -mtune=native -Wall
LFLAGS = -lm -lcblas
APPS = $(wildcard apps/*.c)
RELEASE = $(patsubst apps/%.c, bin/%, $(APPS))
DEBUG = $(patsubst apps/%.c, bin/%_debug, $(APPS))
PROFILE = $(patsubst apps/%.c, bin/%_profile, $(APPS))

all: release debug profile

release: $(RELEASE)

debug: FLAGS += -DDEBUG -g -fsanitize=undefined #-fsanitize=address
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
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS)

bin/%_debug: %.o bin/shray_debug.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS)

bin/%_profile: %.o bin/shray_profile.o
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS)

clean:
	$(RM) bin/*
