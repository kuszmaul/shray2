For the application programmer.

Usage
=========================================================================

The minimal Shray program is 

#include "shray.h"

int main(int argc, char **argv)
{
    size_t cacheSize = 4096000;

    ShrayInit(&argc, &argv, cacheSize);

    ShrayFinalize();

    return 0;
}

where cacheSize is the number of bytes you want to make available on each node for 
Shray, on top of the array allocations.

Compile such a program with mpicc and run with 

# mpirun -n P ./myProgram

where P is the desired number of nodes.

Inside a Shray program, between ShrayInit and ShrayFinalize,
one may do array-computations in parallel.
To write to an array A of size n1 x ... x nd in parallel,
one should first call

ShrayMalloc(n1, sizeof(A));

to allocate the desired memory.

Then compute A from A[ShrayStart(n1)] to A[ShrayEnd(n1)]

Before the result is available for reading, one should call

ShraySync(A)

When you do not need the memory anymore, call

ShrayFree(A)

Profiling
=========================================================================

To time a function, one may use the macro SHRAY_TIME(functionCall), which 
prints the name of the function and the time in seconds to stderr.
