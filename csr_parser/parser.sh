#!/bin/bash

set -eu

# Usage: file name, number of processors

if [ "$#" -lt 3 ]; then
	printf "Usage: [FILENAME] [NUMBER OF PROCESSORS] [PARSER BINARY]\n" >&2
	exit 1
fi

#make

filename="$1"
nproc="$2"
parser="$3"

sed -i '/^%/d' "$filename"

(head -n 1 "$filename"; tail -n +2 "$filename" | sort -n -k 1) >temp
mv temp "$filename"

"$parser" "$filename" "$nproc"
