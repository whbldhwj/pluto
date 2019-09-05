#include "stdio.h"
#include "stdlib.h"
#include "math.h"

typedef int data_t;
#define I 8 
#define J 8 
#define K 8 

void dsa_kernel(data_t A[I + 1][K + 1], data_t B[K + 1][J + 1], data_t C[I + 1][J + 1]);
