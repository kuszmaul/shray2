#!/bin/sh

set -eu

hash sed
hash sbatch
hash basename
hash dirname
hash find
hash gnuplot
hash head
hash seq
hash m4
hash grep

if [ "$#" -ne 3 ]; then
	printf "Usage: benchmark.sh BINDIR OUTPUTDIR SCRIPTDIR\n\n" >&2
	printf "\tBINDIR:    Build directory as generated by CMake.\n" >&2
	printf "\tOUTPUTDIR: Directory to store benchmark results.\n" >&2
	printf "\tSCRIPTDIR: Directory where additional scripts are located.\n" >&2
	exit 1
fi

bindir="$1"
outputdir="$2"
scriptdir="$3"

curdate=$(date -u '+%Y-%m-%dT%H:%M:%S+00:00')
datadir="$outputdir/$curdate"
mkdir -p "$datadir"

scriptsdir="$datadir/scripts"
mkdir -p "$scriptsdir"

printf 'Running benchmarks with output directory '\''%s'\''\n' \
	"$datadir"

# Obtain system information
printf 'Obtaining system configuration\n'
if ! sbatch --quiet --wait ./systeminfo.sh "$datadir"; then
	printf 'Error while running systeminfo.sh via sbatch\n' >&2
	exit 1
fi

# Sleep to wait for SLURM to finish writing to the output file.
sleep 5
rm ./systeminfo.txt

# Cluster configuration
max_ntasks_per_node=8
max_ntasks=64

# Generate scripts and run them
for threadtype in single multi; do
	ntasks=1
	ntasks_per_node=1
	while [ "$ntasks" -le "$max_ntasks" ]; do
		if [ "$ntasks" -le "$max_ntasks_per_node" ]; then
			nodes=1
			ntasks_per_node="$ntasks"
		else
			nodes=$((ntasks / max_ntasks_per_node))
			ntasks_per_node="$max_ntasks_per_node"
		fi

		name="./benchmark.$threadtype.$nodes.$ntasks"
		gen_script="$name.sh"
		gen_output="$name.txt"
		m4 -EP \
			-D "__NTASKS__=[[[$ntasks]]]" \
			-D "__NODES__=[[[$nodes]]]" \
			-D "__THREADTYPE__=[[[$threadtype]]]" \
			-D "__NTASKS_PER_NODE__=[[[$ntasks_per_node]]]" \
			-- ./benchmark_run.sh >"$gen_script"

		printf 'Running configuration (%s tasks/threads, %s nodes, %s tasks/threads per node, %s)\n' \
			"$ntasks" \
			"$nodes" \
			"$ntasks_per_node" \
			"$threadtype"
		if ! sbatch --quiet --wait "$gen_script" "$bindir" "$datadir" "$scriptdir"; then
			printf 'Error while running %s via sbatch\n' "$gen_script" >&2
			exit 1
		fi

		# Sleep to wait for SLURM to finish writing to the output file.
		sleep 5

		if failed=$(grep -c 'result: FAILED' "$gen_output"); then
			printf '\tFAILED: %s\n' \
				"$failed"
		fi

		mv "$gen_script" "$scriptsdir"
		mv "$gen_output" "$scriptsdir"

		ntasks=$(( 2 * ntasks ))
	done
done

# Generate plots.
mkdir -p "$datadir/plots/single"
mkdir -p "$datadir/plots/multi"
for thread_type in single multi; do
	for exp in "$datadir/$thread_type"/*; do
		example=$(basename "$exp")

		# Go through all parameters used for a benchmark.
		for paramsdir in "$exp"/*; do
			params=$(basename "$paramsdir")

			# Go through all implementations of that benchmark.
			for implementation in "$paramsdir"/*; do
				implementation=$(basename "$implementation")
				resultdir="$paramsdir/$implementation"
				[ -f "$resultdir/graph.data" ] && rm "$resultdir/graph.data"

				# Group the data per number of processors.
				find "$resultdir/1" -name '*.gflops' -exec basename -s '.gflops' {} \; \
					| sort -n \
					| while IFS= read -r nproc; do
					for testrun in "$resultdir"/*; do
						if [ ! -d "$testrun" ]; then
							continue
						fi
						result=$(cat "$testrun/$nproc.gflops")
						printf '%s\n' "$result" >>"$resultdir/$nproc.raw"
					done
				done

				# Generate the data for the graphs.
				find "$resultdir" -name '*.raw' -exec basename {} \; \
					| sort -n \
					| while IFS= read -r nproc_result; do
					nproc=$(basename "$nproc_result" .raw)
					python3 calc.py "$resultdir/$nproc_result" "$resultdir/$nproc.result"
					result=$(cat "$resultdir/$nproc.result")
					printf '%s, %s\n' "$nproc" "$result" >>"$resultdir/graph.data"
				done
			done

			# Generate the plot for this specific benchmark.
			filtered=$(printf '%s' "$example" | tr _ ' ')
			if [ "$thread_type" = multi ]; then
				xaxis="Number of threads"
			else
				xaxis="Number of ranks"
			fi
			gnuplot -c benchmark_gflops.gpi \
				"$filtered ($params)" \
				"$datadir/plots/$thread_type" \
				"$paramsdir/chapel/graph.data" \
				"$paramsdir/fortran/graph.data" \
				"$paramsdir/globalarrays/graph.data" \
				"$paramsdir/shray/graph.data" \
				"$paramsdir/scala/graph.data" \
				"$paramsdir/upc/graph.data" \
				"$xaxis"
		done
	done
done
