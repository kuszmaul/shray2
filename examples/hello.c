#include <shray2/shray.h>

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    printf("Hello world!\n");

    ShrayReport();

    ShrayFinalize(0);
}
