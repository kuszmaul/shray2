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

# General.
hash basename
hash dirname
hash find
hash gnuplot
hash head
hash mpirun # General MPI applications (e.g. fortran/globalarrays)

# Chapel.
hash chpl

# Fortran.
hash gfortran

# GASNet.
hash gasnet_trace
hash gasnetrun_mpi

# GlobalArrays
hash ga-config

# UPC.
hash upcc
hash upcrun

if [ "$#" -lt 2 ]; then
	printf "Usage: BINDIR OUTPUTDIR\n" >&2
	exit 1
fi

export SHRAY_CACHELINE=1
export SHRAY_WORKERTHREADS=0

bindir="$1"
outputdir="$2"
failed=0

# Create the new data directory.
curdate=$(date -u '+%Y-%m-%dT%H:%M:%S+00:00')
datadir="$outputdir/$curdate"
mkdir -p "$datadir"

printf 'Running benchmarks with output directory '\''%s'\''\n' \
	"$datadir"

# Create a file with the system parameters.
{
	printf 'Benchmark system configuration on %s\n' "$curdate"

	printf '\n----------\nChapel:\n'
	chpl --version

	printf '\n----------\nFortran:\n'
	gfortran --version

	printf '\n----------\nGlobalArrays:\n'
	ga-config --version

	printf '\n----------\nMPI:\n'
	mpirun --version

	printf '\n----------\nGASNet:\n'
	# gasnet_trace exits with an error code on help messages, gasnetrun_mpi
	# does not support a version argument.
	if gasnet_trace -h; then
		true
	fi

	printf '\n----------\nUPC:\n'
	upcc --network=mpi --version
}>"$datadir/system.txt"

# Run the given test, writes 0 to the gflops file if the test fails.
# $1: output file name
# $1: gflops file name
# $2: complete test invocation
runtest_wrapper()
{
	outfile="$1"
	gflopsfile="$2"
	shift
	shift

	printf '\n\tinvocation: %s\n\toutput file: %s\n\tresult: ' \
		"$*" \
		"$outfile"

	if ! "$@" >"$gflopsfile" 2>"$outfile"; then
		printf '0.0\n' >"$gflopsfile"
		printf 'FAILED\n'
		failed=$((failed + 1))
	else
		printf 'SUCCESS\n'
	fi
}

# Run a single test program for the given type.
# $1: test type
# $2: number of processors
# $3: executable
# $4: arguments to the program WITHOUT special strings.
# $5: arguments to the program as it expects them
runtest()
{
	test_type="$1"
	nproc="$2"
	example="$3"
	argsstr="$4"
	args="$5"

	printf '%s: ' "$test_type"

	resultdir="$datadir/$example/$argsstr/$test_type"
	mkdir -p "$resultdir"

	outfile="$resultdir/$nproc.out"
	gflopsfile="$resultdir/$nproc.gflops"

	if [ "$test_type" = shray ]; then
		runtest_wrapper "$outfile" "$gflopsfile" \
			gasnetrun_mpi \
			-n "$nproc" \
			"$bindir/examples/$test_type/${example}_profile_${test_type}" \
			"$args"
	elif [ "$test_type" = chapel ]; then
		# TODO: must call the executable itself and use -nl for the
		# number of locales, but this requires a proper setup.
		runtest_wrapper "$outfile" "$gflopsfile" \
			mpirun \
			-n "$nproc" \
			"$bindir/examples/$test_type/${example}_$test_type" \
			"$args"
		# TODO: since mpirun does not properly spawn multiple nodes for
		# chapel we get duplicate results. Only grab the first line.
		head -n 1 -q -- "$gflopsfile" >"$resultdir/tmp"
		mv -- "$resultdir/tmp" "$gflopsfile"
	elif [ "$test_type" = fortran ]; then
		runtest_wrapper "$outfile" "$gflopsfile" \
			mpirun \
			-n "$nproc" \
			"$bindir/examples/$test_type/${example}_$test_type" \
			"$args"
	elif [ "$test_type" = globalarrays ]; then
		runtest_wrapper "$outfile" "$gflopsfile" \
			mpirun \
			-n "$nproc" \
			"$bindir/examples/$test_type/${example}_$test_type" \
			"$args"
	elif [ "$test_type" = upc ]; then
		runtest_wrapper "$outfile" "$gflopsfile" \
			upcrun \
			-n "$nproc" \
			-bind-threads \
			"$bindir/examples/$test_type/${example}_$test_type" \
			"$args"
		# Filter out UPC-specific stderr messages
		sed -i '/^UPCR:/d' "$gflopsfile"
	else
		printf 'Unknown test type: %s' "$test_type" >&2
		exit 1
	fi
}

# Run all tests.
for nproc in 2 4 8; do
	# Matrix.
	for size in 2000 4000 8000; do
		printf '\nBenchmark matrix multiplication (%s nodes, %s x %s)\n' \
			"$nproc" \
			"$size" \
			"$size"

		runtest chapel "$nproc" matrix "$size" "--n=$size"
		runtest fortran "$nproc" matrix "$size" "$size"
		runtest globalarrays "$nproc" matrix "$size" "$size"
		runtest upc "$nproc" matrix "$size" "$size"

		cachesize=$((2 * size * size * 3 * 8 / nproc))
		export SHRAY_CACHESIZE="$cachesize"
		runtest shray "$nproc" matrix "$size" "$size"
	done

	# Sparse matrix-vector multiplication
	# TODO: think about each file since it needs to be prepocessed for nproc
	#for matrix in cage3.mtx cage9.mtx cage11.mtx cage15.mtx; do
	#	for iterations in 10 20; do
	#		#runtest chapel "$nproc" spmv "--fileName='$matrix' --iterations=$iterations"
	#		#runtest fortran "$nproc" spmv "$matrix" "$iterations"
	#		#runtest globalarrays "$nproc" spmv "$matrix" "$iterations"
	#		#runtest upc "$nproc" spmv "$matrix" "$iterations"

	#		cachesize=$((2 * 5154859 / nproc))
	#		export SHRAY_CACHESIZE="$cachesize"
	#		runtest shray "$nproc" spmv "$matrix" "$iterations"
	#	done
	#done
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
		for implementation in "$paramsdir"/*; do
			implementation=$(basename "$implementation")
			resultdir="$paramsdir/$implementation"
			[ -f "$resultdir/graph.data" ] && rm "$resultdir/graph.data"

			# Group the data per number of processors.
			find "$resultdir" -name '*.gflops' -exec basename {} \; \
				| sort -n \
				| while IFS= read -r nproc_result; do
				nproc=$(basename "$nproc_result" .gflops)
				result=$(cat "$resultdir/$nproc_result")
				printf '%s, %s\n' "$nproc" "$result" >>"$resultdir/graph.data"
			done
		done

		# Generate the plot for this specific benchmark.
		gnuplot -c benchmark_gflops.gpi \
			"$example ($params)" \
			"$datadir/plots" \
			"$paramsdir/chapel/graph.data" \
			"$paramsdir/fortran/graph.data" \
			"$paramsdir/globalarrays/graph.data" \
			"$paramsdir/shray/graph.data" \
			"$paramsdir/upc/graph.data"
	done
done

if [ "$failed" -ne 0 ]; then
	printf '\nNOTE: %s benchmarks failed during execution. A value of 0.0 will be used in the generated graphs.\n' \
		"$failed" >&2
fi
