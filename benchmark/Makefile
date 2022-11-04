include /usr/local/gasnet/include/mpi-conduit/mpi-seq.mak

FLAGS = -pg
GASNET_PROGRAMS = $(wildcard gasnet/*.c)
GASNET = $(patsubst gasnet/%.c, bin/%_gasnet, $(GASNET_PROGRAMS))

all: $(GASNET)

%.o: gasnet/%.c
	$(GASNET_CC) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $(FLAGS) -c $< -o $@

bin/%_gasnet: %.o 
	$(GASNET_LD) $(GASNET_LDFLAGS) $^ -o $@ $(GASNET_LIBS) $(LFLAGS)

clean:
	$(RM) bin/*
