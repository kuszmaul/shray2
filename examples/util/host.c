#include "host.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define HOSTNAME_LENGTH 256

void hostname_print(void)
{
	char host[HOSTNAME_LENGTH];

	if(gethostname(host, HOSTNAME_LENGTH) != 0) {
		host[0] = '\0';
	}

	fprintf(stderr, "Running on host %s\n", host);
}
