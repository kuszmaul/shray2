#!/bin/sh

NODES=1
while [ $NODES -le 8 ]
do
    sed "s/NODES/$NODES/g" sync.sh.template > sync_"$NODES".sh
    sbatch sync_"$NODES".sh
    rm sync_"$NODES".sh

    NODES=$(( 2 * NODES ))
done
