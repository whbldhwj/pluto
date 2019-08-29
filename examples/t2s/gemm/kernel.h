#include "stdio.h"
#include "stdlib.h"
#include "math.h"

typedef float data_t;
#define I 8 
#define J 8 
#define K 8 

void dsa_kernel(data_t A[I][K], data_t B[K][J], data_t C[I][J]);
