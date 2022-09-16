all: minimalO minimalM

minimalO: minimal.c
	mpicc.openmpi $^ -o $@

minimalM: minimal.c
	mpicc.mpich $^ -o $@

clean:
	$(RM) minimalO minimalM
