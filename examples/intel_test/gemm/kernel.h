#include "stdio.h"
#include "stdlib.h"
#include "math.h"

typedef int data_t;
#define I 16 
#define J 16
#define K 16

void dsa_kernel(data_t A[I][K], data_t B[K][J], data_t C[I][J]);
