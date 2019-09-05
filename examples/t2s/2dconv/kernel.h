#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef int data_t;
#define R 8
#define S 8

void dsa_kernel(data_t X[R + 2 + 1][S + 2 + 1], data_t W[3 + 1][3 + 1], data_t Z[R + 1][S + 1]);
