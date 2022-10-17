# Usage: name of executable, number of processors, commandline arguments (maximum of 3).
# Makes a graph of the segfault page numbers on the y-axis, and time on the x-axis

#!/bin/bash

make graph
echo "time,pageNumber" > $1_segfaults.out
mpirun -n $2 bin/$1_graph $3 $4 $5 2>> $1_segfaults.out

sed -i '/WARNING/d' $1_segfaults.out
sed -i '/Shray/d' $1_segfaults.out

# TODO generate plot
