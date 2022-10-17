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

printf 'Running %s (%s proc) with arguments: ' "$example" "$nproc"
for i in "$@"; do
	printf '%s ' "$i"
done
printf '\n'

datadir=experiments/results
mkdir -p "$datadir"

base=$(basename "$example")
datafile="$datadir/${base}_segfaults.data"

mpirun -n "$nproc" "$example" "$@" 2>"$datafile"

sed -i '/WARNING/d' "$datafile"
sed -i '/Shray/d' "$datafile"

gnuplot -c experiments/segfault_time.gpi "$base" "$datafile" "$datadir"
