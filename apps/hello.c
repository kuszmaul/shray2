#include "../include/shray.h"

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv, 4096000);
    printf("Hello world!\n");
    ShrayFinalize();
}
