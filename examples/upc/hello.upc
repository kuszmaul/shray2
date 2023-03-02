#include <upc.h>
#include <stdio.h>

int main()
{
	printf("Thread %d of %d: hello UPC world\n", MYTHREAD, THREADS);
	return 0;
}
