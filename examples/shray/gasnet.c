#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <gasnet.h>

/* Takes care of error handling for GASNet functions that
 * return a success value. */
#define GASNET_SAFE(fncall)                                                              \
    {                                                                                    \
        int retval;                                                                      \
        if ((retval = fncall) != GASNET_OK) {                                            \
            printf("Error during GASNet call\n");                                         \
        }                                                                               \
    }

void init(int *argc, char ***argv)
{
    GASNET_SAFE(gasnet_init(argc, argv));

    GASNET_SAFE(gasnet_attach(NULL, 0, 4096, 0));
}

int main(int argc, char **argv)
{
    init(&argc, &argv);

    char host[80];
    if(gethostname(host, 80) != 0)
        host[0] = '\0';
    printf("Hello world from %s! node %d/%d running on CPU %d!\n", 
        host, gasnet_mynode(), gasnet_nodes(), sched_getcpu());

    gasnet_exit(0);
}
