#!/bin/bash

set -eu

if [ "$#" -lt 1 ]; then
	printf "Usage: [SCALAPACK DIRECTORY]\n" >&2
	printf "Make sure you have downloaded scalapack from
    https://netlib.org/scalapack/#_scalapack_version_2_2_0 into that directory, and
    fill in SLmake.inc in this directory\n" >&2
	exit 1
fi

scaladir="$1"

cp SLmake.inc "${scaladir}"
cp pdtrord.f "${scaladir}/SRC"
cp Makefile "${scaladir}"
cp pdblas3tim.f "${scaladir}/PBLAS/TIMING"
cd "${scaladir}"
make
cd PBLAS/TIMING
make xdpblas3tim
