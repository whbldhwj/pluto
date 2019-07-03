#include "stdio.h"
#include "stdlib.h"
#include "math.h"

typedef float data_t;
#define R 32
#define S 32

void dsa_kernel(data_t X[R + 2][S + 2], data_t W[3][3], data_t Z[R][S]);
