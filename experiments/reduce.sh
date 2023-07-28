#!/bin/sh

if [ "$#" -ne 1 ]; then
    printf 'Usage: number of pages per get\n'
fi

NODES=2
PAGE="$1"
while [ $NODES -le 8 ]
do
    sed "s/NODES/$NODES/g" reduce.sh.template > reduce_"$NODES".sh
    sed -i "s/PAGE/$PAGE/g" reduce_"$NODES".sh
    sbatch reduce_"$NODES".sh

    NODES=$(( 2 * NODES ))
done
