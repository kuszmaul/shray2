#!/bin/sh

# Splits up the sparse matrix files of a certain CG class into multiple files,
# one for each processor in a block distribution. Each processor gets a suffix
# of 2 digits.

set -eu

hash split
hash wc

if [ "$#" -ne 2 ]; then
    printf "Usage: cg_dist.sh CLASS RANKS\n" >&2
    printf "Assumes makea has generated the full files already\n" >&2
fi

distribute()
{
    filename="$1"
    ranks="$2"
    wordsize="$3"
    bytes=$(wc -c < "$1")
    words=$(( bytes / wordsize ))
    blocksize=$(( ((words + ranks - 1) / ranks) * wordsize ))

    split -d --bytes="$blocksize" "$filename" "$filename"
}

distribute "a.cg.$1" "$2" 8
distribute "colidx.cg.$1" "$2" 4
distribute "rowstr.cg.$1" "$2" 4
