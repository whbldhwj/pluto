#include "stdio.h"
#include "stdlib.h"
#include "math.h"

typedef float data_t;
#define I 64
#define J 64
#define K 64

void dsa_kernel(data_t A[I][K], data_t B[K][J], data_t C[I][J]);
