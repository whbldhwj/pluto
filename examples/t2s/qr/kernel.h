#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef float data_t;
#define M 4
#define N 4

void dsa_kernel(data_t A[M][N], data_t Q[M][M], data_t R[M][N]);
