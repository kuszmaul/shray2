#!/bin/sh

#SBATCH --account=csmpi
#SBATCH --nodes=1
#SBATCH --partition=csmpi_long
#SBATCH --time=00:10:00
#SBATCH --output=systeminfo.txt

set -eu

hash chpl
hash gfortran
hash gasnet_trace
hash ga-config
hash upcc

if [ "$#" -lt 1 ]; then
	printf "Usage: systeminfo.sh OUTPUTDIR\n\n" >&2
	printf "\tOUTPUTDIR: Directory to store sytem info.\n" >&2
	exit 1
fi

datadir="$1"

# Create a file with the system parameters.
{
	printf 'Benchmark system configuration\n'

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
	upcc --network=udp --version
}>"$datadir/system.txt"
