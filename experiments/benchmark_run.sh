m4_changecom()m4_dnl
m4_changequote(`[[[',`]]]')m4_dnl
#!/bin/sh

#SBATCH --account=csmpi
#SBATCH --nodes=__NODES__
m4_ifelse(__THREADTYPE__, multi, [[[m4_dnl
#SBATCH --cpus-per-task=__NTASKS_PER_NODE__
#SBATCH --ntasks-per-node=1
]]], [[[m4_dnl
#SBATCH --ntasks=__NTASKS__
#SBATCH --ntasks-per-node=__NTASKS_PER_NODE__
#SBATCH --threads-per-core=1
]]])m4_dnl
#SBATCH --partition=csmpi_long
#SBATCH --time=08:00:00
#SBATCH --output=benchmark.__THREADTYPE__.__NODES__.__NTASKS__.txt

# A script to automatically benchmark test programs.

set -eu

# Ensure required programs exist.

# General.
hash mpirun.openmpi
hash mpirun.mpich
hash seq
hash realpath

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

# UDP gasnet conduit
hash amudprun

if [ "$#" -ne 3 ]; then
	printf "Usage: benchmark_run.sh BINDIR OUTPUTDIR SCRIPTDIR\n\n" >&2
	printf "\tBINDIR:         Build directory as generated by CMake.\n" >&2
	printf "\tOUTPUTDIR:      Directory to store benchmark results.\n" >&2
	printf "\tSCRIPTDIR:      Directory where additional scripts are located.\n" >&2
	exit 1
fi

bindir="$1"
datadir="$2"
scriptdir="$3"

# Run the given test. Writes 0 to the gflops file if the test fails.
# $1: output file name
# $1: gflops file name
# remainder: complete test invocation
runtest_wrapper()
{
	outfile="$1"
	gflopsfile="$2"
	shift 2

	printf '\n\tinvocation: %s\n\toutput file: %s\n\tresult: ' \
		"$*" \
		"$outfile"

	tstart=$(date -u '+%Y-%m-%dT%H:%M:%S+00:00')
	if ! "$@" >"$gflopsfile" 2>"$outfile"; then
		printf '0.0\n' >"$gflopsfile"
		printf 'FAILED\n'
	else
		printf 'SUCCESS\n'
	fi
	tend=$(date -u '+%Y-%m-%dT%H:%M:%S+00:00')
	printf '\ttime: (%s, %s)\n' "$tstart" "$tend"
}

# Run a single test program for the given type.
# $1: test type
# $2: executable
# $3: arguments to the program WITHOUT special strings.
# $4: test number
# remainder: arguments to the program as it expects them
runtest()
{
	test_type="$1"
	example="$2"
	argsstr="$3"
	testnumber="$4"
	shift 4

	printf '%s: ' "$test_type"

	testdir="$datadir/__THREADTYPE__/$example/$argsstr/$test_type"
	resultdir="$testdir/$testnumber"
	mkdir -p "$resultdir"

	outfile="$resultdir/__NTASKS__.out"
	gflopsfile="$resultdir/__NTASKS__.gflops"

m4_ifelse(__THREADTYPE__, multi, [[[m4_dnl
	unset SHRAY_PAR
]]], [[[m4_dnl
	export SHRAY_PAR=N
]]])m4_dnl

	if [ "$test_type" = shray ]; then
		runtest_wrapper "$outfile" "$gflopsfile" \
			mpirun.openmpi m4_ifelse(__THREADTYPE__, multi, --bind-to none) \
			"$bindir/examples/$test_type/${example}_profile_${test_type}" \
			"$@"
	elif [ "$test_type" = chapel ]; then
		runtest_wrapper "$outfile" "$gflopsfile" \
			"$bindir/examples/$test_type/${example}_$test_type" \
			-nl "__NODES__" \
			"$@"
	elif [ "$test_type" = fortran ]; then
		runtest_wrapper "$outfile" "$gflopsfile" \
			mpirun.mpich m4_ifelse(__THREADTYPE__, multi, --bind-to none) \
			"$bindir/examples/$test_type/${example}_$test_type" \
			"$@"
	elif [ "$test_type" = globalarrays ]; then
		runtest_wrapper "$outfile" "$gflopsfile" \
			mpirun.mpich m4_ifelse(__THREADTYPE__, multi, --bind-to none) \
			"$bindir/examples/$test_type/${example}_$test_type" \
			"$@"
	elif [ "$test_type" = upc ]; then
		runtest_wrapper "$outfile" "$gflopsfile" \
			mpirun.openmpi m4_ifelse(__THREADTYPE__, multi, --bind-to none) \
			"$bindir/examples/$test_type/${example}_$test_type" \
			"$@"
		# Filter out UPC-specific stderr messages
		sed -i '/^UPCR:/d' "$gflopsfile"
	elif [ "$test_type" = scala ]; then
		"$scriptdir/scalapack_generator.sh" ${ntasks} 250 "$@"
		runtest_wrapper "$outfile" "$gflopsfile" \
			mpirun m4_ifelse(__THREADTYPE__, multi, --bind-to none) \
			xdpblas3tim
	else
		printf 'Unknown test type: %s' "$test_type" >&2
		exit 1
	fi
}

## Inform GASNet how to start.
export GASNET_SPAWNFN="C"
export GASNET_CSPAWN_CMD="srun -N %N %C"

export CHPL_RT_NUM_THREADS_PER_LOCALE="__NTASKS_PER_NODE__"

## UDP-conduit max timeout in microseconds, default is 30000000 (30 seconds)
## 0 is infinite timeout.
export GASNET_REQUESTTIMEOUT_MAX=0

# UPC shared heap configuration
export UPC_SHARED_HEAP_SIZE="4G"

# Generate matrix required for CG benchmark.
cgclass="A"
cgdatadir="$datadir/cgdata"
mkdir -p "$cgdatadir"
bindirabs=$(realpath -s "$bindir")
scriptdirabs=$(realpath -s "$scriptdir")
curdir="$PWD"
cd "$cgdatadir"
"$bindirabs/examples/util/makea" "$cgclass"
cd "$curdir"

m4_ifelse(__THREADTYPE__, multi, [[[m4_dnl
export OMP_NUM_THREADS="__NTASKS_PER_NODE__"
]]], [[[m4_dnl
export OMP_NUM_THREADS="1"
]]])m4_dnl

# Run all tests.
export SHRAY_CACHEFACTOR=1
for i in $(seq 1 5); do
	# 1D stencil.
	for size in 20480000; do # 40960000; do
		for iter in 1000; do #2000; do
			printf '\nBenchmark 1D 3-point stencil (run %s, %s, %s tasks, %s nodes, %s size, %s iter)\n' \
				"$i" \
				"__THREADTYPE__" \
				"__NTASKS__" \
				"__NODES__" \
				"$size" \
				"$iter"

m4_ifelse(__THREADTYPE__, multi, [[[m4_dnl
			runtest shray 1dstencil_mt "$size $iter" "$i" "$size" "$iter"
			runtest globalarrays 1dstencil_mt "$size $iter" "$i" "$size" "$iter"
			runtest fortran 1dstencil_mt "$size $iter" "$i" "$size" "$iter"
			runtest chapel 1dstencil "$size $iter" "$i" "--N=$size" "--ITERATIONS=$iter"
]]], [[[m4_dnl
			runtest shray 1dstencil "$size $iter" "$i" "$size" "$iter"
			runtest globalarrays 1dstencil "$size $iter" "$i" "$size" "$iter"
			runtest fortran 1dstencil "$size $iter" "$i" "$size" "$iter"
			runtest upc 1dstencil "$size $iter" "$i" "$size" "$iter"
]]])m4_dnl
		done
	done

	# Sparse matrix-vector multiplication (monopoly)
	for size in 204800; do # 20480000; do
		for iterations in 5; do
			printf '\nBenchmark spmv monopoly (run %s, %s, %s tasks, %s nodes, %s x %s, %s iter)\n' \
				"$i" \
				"__THREADTYPE__" \
				"__NTASKS__" \
				"__NODES__" \
				"$size" \
				"$size" \
				"$iterations"

m4_ifelse(__THREADTYPE__, multi, [[[m4_dnl
			runtest shray monopoly_mt "$size $iterations" "$i" "$size" "$iterations"
			runtest globalarrays monopoly_mt "$size $iterations" "$i" "$size" "$iterations"
			runtest fortran monopoly_mt "$size $iterations" "$i" "$size" "$iterations"
			runtest chapel monopoly "$size $iterations" "$i" "--n=$size" "--iterations=$iterations"
]]], [[[m4_dnl
			runtest shray monopoly "$size $iterations" "$i" "$size" "$iterations"
			runtest globalarrays monopoly "$size $iterations" "$i" "$size" "$iterations"
			runtest fortran monopoly "$size $iterations" "$i" "$size" "$iterations"
			runtest upc monopoly "$size $iterations" "$i" "$size" "$iterations"
]]])m4_dnl
		done
	done

	# Matrix, weak scaling.
	testdirname="2146 3036 4296 6080 8608 12192 17280"
	case "__NTASKS__" in
		 1) size=2146;;
		 2) size=3036;;
		 4) size=4296;;
		 8) size=6080;;
		16) size=8608;;
		32) size=12192;;
		64) size=17280;;
	esac
	printf '\nBenchmark matrix multiplcation (run %s, %s, %s tasks, %s nodes, %s x %s )\n' \
		"$i" \
		"__THREADTYPE__" \
		"__NTASKS__" \
		"__NODES__" \
		"$size" \
		"$size"

m4_ifelse(__THREADTYPE__, multi, [[[m4_dnl
	export BLIS_NUM_THREADS="__NTASKS_PER_NODE__"
	export OPENBLAS_NUM_THREADS="__NTASKS_PER_NODE__"
	runtest shray matrix "$testdirname" "$i" "$size"
	runtest globalarrays matrix "$testdirname" "$i" "$size"
	runtest fortran matrix "$testdirname" "$i" "$size"
	runtest chapel matrix "$testdirname" "$i" "--n=$size"
]]], [[[m4_dnl
	export BLIS_NUM_THREADS=1
	export OPENBLAS_NUM_THREADS=1
	runtest shray matrix "$testdirname" "$i" "$size"
	runtest globalarrays matrix "$testdirname" "$i" "$size"
	runtest fortran matrix "$testdirname" "$i" "$size"
	runtest upc matrix "$testdirname" "$i" "$size"
	#runtest scala matrix "$testdirname" "$i" "$size"
]]])m4_dnl

	# CG.
	printf '\nBenchmark CG (run %s, %s, %s tasks, %s nodes, class %s )\n' \
		"$i" \
		"__THREADTYPE__" \
		"__NTASKS__" \
		"__NODES__" \
		"$cgclass"
m4_ifelse(__THREADTYPE__, multi, [[[m4_dnl
	runtest shray cg "$cgclass" "$i" "$cgclass" "$cgdatadir"
]]], [[[m4_dnl
	runtest shray cg "$cgclass" "$i" "$cgclass" "$cgdatadir"
]]])m4_dnl

done

# Clean up CG data.
rm -r "$cgdatadir"
