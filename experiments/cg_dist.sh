#!/bin/sh

set -eu

hash sed
hash wc

if [ "$#" -lt 2 ]; then
    printf "Usage: cg_dist.sh CLASS RANKS\n" >&2
           "Assumes makea has generated the full files already\n" >&2
fi

distribute()
{
    filename="$1"
    ranks="$2"
    lines=$(wc -l < "$1")

    i=0

    while [ "$i" -lt "$ranks" ]; do
        start=$(( i * (lines + ranks - 1) / ranks + 1 ))
        end=$(( (i + 1) * (lines + ranks - 1) / ranks ))
        # Inefficient, if this becomes a problem I can write something like
        # split, but with block distribution
        sed -n "$start,${end}p;${end}q" "$filename" > "${filename}_${i}"
        i=$((i + 1))
    done
}

distribute "a.cg.$1" "$2"
distribute "colidx.cg.$1" "$2"
distribute "rowstr.cg.$1" "$2"
