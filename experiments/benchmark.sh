#!/bin/sh

# A script to automatically benchmark test programs.

set -eu

# Ensure required programs exist.
hash gasnet_trace
hash gnuplot
hash mpirun
hash oshrun
hash time

if [ "$#" -lt 1 ]; then
	printf "Usage: BUILDDIR\n" >&2
	exit 1
fi

builddir="$1"

# Run a single test program using Shray.
# $1: executable
# $2: number of processors
# remainder: arguments to the program
shraytest()
{
	nproc="$1"
	example="$2"
	shift
	shift

	printf 'Running %s (%s proc, shray) with arguments: %s\n' \
		"$example" \
		"$nproc" \
		"$*"

	resultdir="$datadir/$example/shray"
	mkdir -p "$resultdir"

	base=$(basename "$example")
	outfile="$resultdir/${base}_${nproc}_{$*}.out"
	timefile="$resultdir/${base}_${nproc}_{$*}.time"

	command time \
		-o "$timefile" \
		-f '%e' \
		-- mpirun -n "$nproc" "$builddir/${example}_profile" "$@" 2> "$outfile"

	#gnuplot -c experiments/segfault_time.gpi "$base" "$datafile" "$datadir"
}

# Create the new data directory.
curdate=$(date -u '+%Y-%m-%dT%H:%M:%S+00:00')
datadir="benchmarks/$curdate"
mkdir -p "$datadir"

# Create a file with the system parameters.
{
	printf 'Benchmark system configuration on %s\n' "$curdate"
	printf '\nMPI:\n'
	mpirun --version
	if hash ompi_info 2>/dev/null; then
		printf '\nOpenMPI:\n'
		ompi_info
	fi
	if hash mpichversion 2>/dev/null; then
		printf '\nMPICH:\n'
		mpichversion
	fi

	printf '\n----------\nGASNet:\n'
	# gasnet_trace exists with an error code on help messages
	if gasnet_trace -h; then
		true
	fi

	printf '\n----------\nOpenSHMEM:\n'
	oshrun -h
}>"$datadir/system.txt"

# Run all tests.
for nproc in 1 2 4 8; do

	# Random access test
	for size in 400000 800000; do
		for probability in 90 95 99; do
			if ! shraytest "$nproc" random "$size" "$probability"; then
				printf '    FAILED\n' >&2
			fi
		done
	done
done
