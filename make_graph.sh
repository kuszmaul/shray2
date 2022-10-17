#!/bin/sh

# Usage: name of executable, number of processors, commandline arguments.
# Makes a graph of the segfault page numbers on the y-axis, and time on the x-axis

set -eu

if [ "$#" -lt 3 ]; then
	printf "Usage: EXECUTABLE NUMBER_PROCESSORS [EXECUTABLE ARGS]\n" >&2
	exit 1
fi

example="$1"
nproc="$2"

shift
shift

printf 'Running %s (%s proc) with arguments: %s\n' "$example" "$nproc" "$@"

datadir=experiments/results
mkdir -p "$datadir"

base=$(basename "$example")
datafile="$datadir/${base}_segfaults.data"

printf 'time,pageNumber\n' >"$datafile"
mpirun -n "$nproc" "$example" "$@" 2>>"$datafile"

sed -i '/WARNING/d' "$datafile"
sed -i '/Shray/d' "$datafile"
sed -i '/time,/d' "$datafile"

# make graph using gnu plot
gnuplot -c experiments/segfault_time.gpi "$base" "$datafile" "$datadir"
