#!/bin/sh

#SBATCH --account=csmpi
#SBATCH --partition=csmpi_short
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=1
#SBATCH --output=bandwidth.out
#SBATCH --time=0:10:00

set -eu

# Ensure required programs exist.
hash time
hash mpirun.mpich

export GASNET_SPAWNFN="C"
export GASNET_CSPAWN_CMD="srun -n %N %C"

bindir="../build/examples"
outputdir="./results"

# Create the new data directory.
curdate=$(date -u '+%Y-%m-%dT%H:%M:%S+00:00')
datadir="$outputdir/$curdate"
mkdir -p "$datadir"

# Create a file with the system parameters.
{
	printf 'Benchmark system configuration on %s\n' "$curdate"

	printf '\nMPI:\n'
	mpirun.mpich --version
	if hash ompi_info 2>/dev/null; then
		printf '\nOpenMPI:\n'
		ompi_info
	fi
	if hash mpichversion 2>/dev/null; then
		printf '\nMPICH:\n'
		mpichversion
	fi
}>"$datadir/system.txt"

runbandwidth()
{
    size="$1"
    printf '%s & ', "${size}";
    "${bindir}/shray/bandwidth_normal_shray" 2 409600000 "${size}" 10;
    printf ' & ';
    mpirun.mpich -n 2 "${bindir}/mpi/bandwidth_mpi" 409600000 "${size}" 10;
    printf ' \\\n';
}

{
printf '\begin{tabular}{c|c|c}\n\hline\nPacket size & Shray & MPI\n';
runbandwidth 4096;
runbandwidth 40960000;
printf '\end{tabular}';
} > "${datadir}/bandwidth.csv" 2> "${datadir}/errors.txt"
