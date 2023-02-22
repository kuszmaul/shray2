#!/bin/sh

#SBATCH --account=csmpi
#SBATCH --partition=csmpi_short
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=8
#SBATCH --output=reduce.out
#SBATCH --time=0:10:00

set -eu

# Ensure required programs exist.
hash time

export GASNET_SPAWNFN="C"
export GASNET_CSPAWN_CMD="srun -n %N %C"

bindir="../build/examples"
outputdir="./results/reduce"

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

runsync()
{
    nproc="$1"
    printf '%s & ' "${nproc}";
    "${bindir}/shray/segfault_normal_shray" "${nproc}" 500000000;
    printf ' \\\\\n';
}

{
printf '\\begin{tabular}{c|c}\n\hline\nProcesses & time per page (us) \\\\\n';
for nproc in 8 16 32 64; do
    runsync "${nproc}"
done
printf '\\end{tabular}';
} > "${datadir}/sync.csv" 2> "${datadir}/errors.txt"
