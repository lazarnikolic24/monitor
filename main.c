#include <stdio.h>
#include <stdlib.h>
#include "testers.h"

int main(int argc, char* argv[]){
    int n = 20;
    if (argc > 1){
        n = atoi(argv[1]);
    }

    int r = incrementer_test(n);
    printf("Incrementer test: got %d, expected %d\n", r, n);

    return 0;
}
