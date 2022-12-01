# Usage: file name, number of processors

#!/bin/bash

if [ "$#" -lt 2 ]; then
	printf "Usage: filename number of processors\n" >&2
	exit 1
fi

make

sed -i '/^%/d' $1

(head -n 1 $1; tail -n +2 $1 | sort -n -k 1) > temp
mv temp $1

./parser $1 $2
