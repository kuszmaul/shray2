#!/bin/sh

#SBATCH -p csedu
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=16
#SBATCH -t 5:00:00

# A script to automatically benchmark test programs.
# For Shray on P processors and problem size n bytes,
# we set the cache size equal to 2n/P.

set -eu

# Ensure required programs exist.
hash gasnet_trace
hash gnuplot
hash mpirun
hash oshrun
hash time
hash find
#hash ga-config

export SHRAY_CACHELINE=1

bindir="../bin"
outputdir="../results"

# Run a single test program using Fortran.
# $1: executable
# $2: number of processors
# remainder: arguments to the program
fortrantest()
{
	nproc="$1"
	example="$2"
	shift
	shift

	printf 'Running %s (%s proc, fortran) with arguments: %s\n' \
		"$example" \
		"$nproc" \
		"$*"

	resultdir="$datadir/$example/$*/fortran"
	mkdir -p "$resultdir"

	outfile="$resultdir/$nproc.out"
	timefile="$resultdir/$nproc.time"

    mpirun -n "$nproc" ${bindir}/fortran/${example}_fortran  "$@" > "$timefile" 2> "$outfile"
}

# Run a single test program using OpenSHMEM.
# $1: executable
# $2: number of processors
# remainder: arguments to the program
oshmemtest()
{
	nproc="$1"
	example="$2"
	shift
	shift

	printf 'Running %s (%s proc, oshmem) with arguments: %s\n' \
		"$example" \
		"$nproc" \
		"$*"

	resultdir="$datadir/$example/$*/oshmem"
	mkdir -p "$resultdir"

	outfile="$resultdir/$nproc.out"
	timefile="$resultdir/$nproc.time"

    oshrun -n "$nproc" ${bindir}/oshmem/${example}_oshmem "$@" > "$timefile" 2> "$outfile"
}

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

	resultdir="$datadir/$example/$*/shray"
	mkdir -p "$resultdir"

	outfile="$resultdir/$nproc.out"
	timefile="$resultdir/$nproc.time"

    mpirun -n "$nproc" ${bindir}/${example}_profile "$@" > "$timefile" 2> "$outfile"
}

shraytestMatrix()
{
    export SHRAY_CACHESIZE=$((2*$3*$3*3*8/$1))
    shraytest $@
}

shraytestBodies()
{
    export SHRAY_CACHESIZE=$((2*80*$3/$1))
    shraytest $@
}

shraytestStencil()
{
    export SHRAY_CACHESIZE=$((2*$3*$3*8/$1))
    shraytest $@
}

shraytestRandom()
{
    export SHRAY_CACHESIZE=4096
    shraytest $@
}

# Run a single test program using GlobalArrays.
# $1: executable
# $2: number of processors
# remainder: arguments to the program
gatest()
{
	nproc="$1"
	example="$2"
	shift
	shift

	printf 'Running %s (%s proc, ga) with arguments: %s\n' \
		"$example" \
		"$nproc" \
		"$*"

	resultdir="$datadir/$example/$*/globalarrays"
	mkdir -p "$resultdir"

	outfile="$resultdir/$nproc.out"
	timefile="$resultdir/$nproc.time"

	# TODO: tmp ucx
	command time \
		-o "$timefile" \
		-f '%e' \
		-- mpirun -n "$nproc" --mca osc ucx \
			"$bindir/globalarrays/${example}_globalarrays" \
			"$@" >"$outfile" 2>&1
}

# Create the new data directory.
curdate=$(date -u '+%Y-%m-%dT%H:%M:%S+00:00')
datadir="$outputdir/$curdate"
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
	# gasnet_trace exits with an error code on help messages
	if gasnet_trace -h; then
		true
	fi

	printf '\n----------\nOpenSHMEM:\n'
	oshrun -h

	printf '\n----------\nGlobalArrays:\n'
#	ga-config --version
}>"$datadir/system.txt"

# Run all tests.
for nproc in 2 4 8 16 32; do
	for test in fortrantest oshmemtest shraytestMatrix; do
		# Matrix.
		for size in 2048 4096 8192; do
			if ! "$test" "$nproc" matrix "$size"; then
				printf '    FAILED\n' >&2
			fi
		done
    done

	for test in fortrantest oshmemtest shraytestBodies; do
		# N-body.
		for bodies in 1024 2048; do
			for iterations in 1 2 5; do
				if ! "$test" "$nproc" nbody "$bodies" "$iterations"; then
					printf '    FAILED\n' >&2
				fi
			done
		done
    done

	for test in fortrantest oshmemtest shraytestStencil; do
		# Stencil.
        # Small because Fortran is very slow
		for n in 1024 2048 4096; do
			for iterations in 1 2 5; do
				if ! "$test" "$nproc" 2dstencil "$n" "$iterations"; then
					printf '    FAILED\n' >&2
				fi
			done
		done
    done

	for test in fortrantest oshmemtest shraytestRandom; do
		# Random access.
		for size in 10000000; do
            for n in 100000; do
		    	for probability in 10 90 99; do
		    		if ! "$test" "$nproc" random "$size" "$n" "$probability"; then
		    			printf '    FAILED\n' >&2
		    		fi
                done
			done
		done
	done
done

# Generate plots.
mkdir -p "$datadir/plots"
for exp in "$datadir"/*; do
	example=$(basename "$exp")

	if [ ! -d "$exp" ] || [ "$example" = plots ]; then
		continue
	fi

	# Go through all parameters used for a benchmark.
	for paramsdir in "$exp"/*; do
		params=$(basename "$paramsdir")

		# Go through all implementations of that benchmark.
		for implementation in shray oshmem fortran; do
			resultdir="$paramsdir/$implementation"
			[ -f "$resultdir/graph.data" ] && rm "$resultdir/graph.data"

			# Group the data per number of processors.
			find "$resultdir" -name '*.time' -exec basename {} \; \
				| sort -n \
				| while IFS= read -r nproc_result; do
				nproc=$(basename "$nproc_result" .time)
				result=$(cat "$resultdir/$nproc_result")
				printf '%s, %s\n' "$nproc" "$result" >>"$resultdir/graph.data"
			done
		done

		# Generate the plot for this specific benchmark.
		gnuplot -c benchmark_time.gpi \
			"$example ($params)" \
			"$datadir/plots" \
			"$paramsdir/fortran/graph.data" \
			"$paramsdir/shray/graph.data" \
			"$paramsdir/oshmem/graph.data"
	done
done
