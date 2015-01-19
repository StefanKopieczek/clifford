#include "clifford.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    cliff_init();

    printf("%d\n", cliff_lte(cliff(10), cliff(10)));

    cliff_teardown();
}
