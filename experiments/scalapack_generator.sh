#!/bin/bash

repeatEcho()
{
    k=$1
    identifier="$2"
    shift; shift;
    message="$@"

    for ((i = 0; i < $k; i++)); do
        printf "${message}\t" >> PDBLAS3TIM.dat
    done

    echo "values of ${identifier}" >> PDBLAS3TIM.dat
}

# Generates a driver file for one specific number of processors, but multiple matrix sizes
# are allowed.

set -eu

if [ "$#" -lt 3 ]; then
    printf "Usage: [NUMBER OF PROCESSORS] [LOGICAL BLOCKSIZE (around 250)] [MATRIX SIZES]\n" >&2
	exit 1
fi

nproc="$1"
block="$2"
shift; shift;
n="$@"
numberOfExperiments=$#

echo "'Level 3 PBLAS, Timing input file'" > PDBLAS3TIM.dat
echo "'Intel iPSC/860 hypercube, gamma model.'
'PDBLAS3TIM.SUMM'	output file name (if any)
0		device out
${block}		value of the logical computational blocksize NB
1 		number of process grids (ordered pairs of P & Q)
${nproc}  	values of P
1  	values of Q
1.0D0		value of ALPHA
0.0D0		value of BETA
${numberOfExperiments}				number of tests problems" >> PDBLAS3TIM.dat

repeatEcho ${numberOfExperiments} "DIAG" "'N'"
repeatEcho ${numberOfExperiments} "SIDE" "'L'"
repeatEcho ${numberOfExperiments} "TRANSA" "'N'"
repeatEcho ${numberOfExperiments} "TRANSB" "'N'"
repeatEcho ${numberOfExperiments} "UPLO" "'U'"

echo "${n} values of M" >> PDBLAS3TIM.dat
echo "${n} values of N" >> PDBLAS3TIM.dat
echo "${n} values of K" >> PDBLAS3TIM.dat
echo "${n} values of M_A" >> PDBLAS3TIM.dat
echo "${n} values of N_A" >> PDBLAS3TIM.dat

repeatEcho ${numberOfExperiments} "IMB_A" "1"
repeatEcho ${numberOfExperiments} "INB_A" "1"
repeatEcho ${numberOfExperiments} "MB_A" "1"
repeatEcho ${numberOfExperiments} "NB_A" "1"
repeatEcho ${numberOfExperiments} "RSRC_A" "0"
repeatEcho ${numberOfExperiments} "CSRC_A" "0"
repeatEcho ${numberOfExperiments} "IA" "1"
repeatEcho ${numberOfExperiments} "JA" "1"

echo "${n} values of M_B" >> PDBLAS3TIM.dat
echo "${n} values of N_B" >> PDBLAS3TIM.dat

repeatEcho ${numberOfExperiments} "IMB_B" "1"
repeatEcho ${numberOfExperiments} "INB_B" "1"
repeatEcho ${numberOfExperiments} "MB_B" "1"
repeatEcho ${numberOfExperiments} "NB_B" "1"
repeatEcho ${numberOfExperiments} "RSRC_B" "0"
repeatEcho ${numberOfExperiments} "CSRC_B" "0"
repeatEcho ${numberOfExperiments} "IB" "1"
repeatEcho ${numberOfExperiments} "JB" "1"

echo "${n} values of M_C" >> PDBLAS3TIM.dat
echo "${n} values of N_C" >> PDBLAS3TIM.dat

repeatEcho ${numberOfExperiments} "IMB_C" "1"
repeatEcho ${numberOfExperiments} "INB_C" "1"
repeatEcho ${numberOfExperiments} "MB_C" "1"
repeatEcho ${numberOfExperiments} "NB_C" "1"
repeatEcho ${numberOfExperiments} "RSRC_C" "0"
repeatEcho ${numberOfExperiments} "CSRC_C" "0"
repeatEcho ${numberOfExperiments} "IC" "1"
repeatEcho ${numberOfExperiments} "JC" "1"

echo "PDGEMM  T	put F for no test in the same column
PDSYMM  F	put F for no test in the same column
PDSYRK  F	put F for no test in the same column
PDSYR2K F	put F for no test in the same column
PDTRMM  F	put F for no test in the same column
PDTRSM  F	put F for no test in the same column
PDGEADD F	put F for no test in the same column
PDTRADD F	put F for no test in the same column" >> PDBLAS3TIM.dat
