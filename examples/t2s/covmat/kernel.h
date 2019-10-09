#include "stdio.h"
#include "stdlib.h"
#include "math.h"

typedef float data_t;
#define N 8 
#define K 8 

void dsa_kernel(data_t d[N + 1][K + 1], data_t V[K + 1][K + 1]);
