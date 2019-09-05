#include "stdio.h"
#include "stdlib.h"
#include "math.h"

typedef float data_t;
#define I 64
#define J 64
#define K 64
#define L 64

void dsa_kernel(data_t A[I][K][L], data_t B[K][J], data_t C[L][J], data_t D[I][J]);
