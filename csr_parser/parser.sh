#!/bin/bash

set -eu

if [ "$#" -lt 3 ]; then
	printf "Usage: [FILENAME] [NUMBER OF PROCESSORS] [RESULT DIRECTORY] called from directory
        containing the parser binary\n" >&2
	exit 1
fi

filename="$1"
nproc="$2"
mmdir="$3"
wd=${PWD}
targetdir=${mmdir}/${nproc}/${filename}

mkdir -p "${targetdir}"

sed -i '/^%/d' "$filename"

(head -n 1 "$filename"; tail -n +2 "$filename" | sort -n -k 1) >temp
mv temp "$filename"

if ! [ -e "${targetdir}/${filename}" ]; then
    ln -s "${wd}/${filename}" "${targetdir}/${filename}"
fi

cd "${targetdir}"

"${wd}/parser" "$filename" "$nproc"
