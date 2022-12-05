#!/bin/sh

# A script to automatically benchmark test programs.
# For Shray on P processors and problem size n bytes,
# we set the cache size equal to 2n/P.

set -eu

# Ensure required programs exist.
hash gasnet_trace
hash gnuplot
hash gasnetrun_mpi
hash time
hash find
#hash ga-config

export SHRAY_CACHELINE=1

if [ "$#" -lt 2 ]; then
	printf "Usage: BINDIR OUTPUTDIR\n" >&2
	exit 1
fi

bindir="$1"
outputdir="$2"

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

# Run a single test program using Fortran.
# $1: executable
# $2: number of processors
# remainder: arguments to the program
chapeltest()
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

    ${bindir}/chapel/${example}  "$@" -nl "$nproc" > "$timefile" 2> "$outfile"
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

    gasnetrun_mpi -n "$nproc" ${bindir}/${example}_profile "$@" > "$timefile" 2> "$outfile"
}

shraytestMatrix()
{
    export SHRAY_CACHESIZE=$((2*$3*$3*3*8/$1))
    shraytest $@
}

shraytestSpmv()
{
    export SHRAY_CACHESIZE=$(8*5154859/$1)
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
for nproc in 2 4; do
	for test in fortrantest oshmemtest shraytestMatrix; do
		# Matrix.
		for size in 2000 4000 8000; do
			if ! "$test" "$nproc" matrix "$size"; then
				printf '    FAILED\n' >&2
			fi
		done
    done

	for test in fortrantest oshmemtest shraytestSpmv; do
		# Sparse matrix-vector multiplication
		for matrix in cage15.mtx; do
			for iterations in 10 20; do
				if ! "$test" "$nproc" spmv "$matrix" "$iterations"; then
					printf '    FAILED\n' >&2
				fi
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
