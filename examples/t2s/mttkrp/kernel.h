#include "stdio.h"
#include "stdlib.h"
#include "math.h"

typedef float data_t;
#define I 8 
#define J 8 
#define K 8 
#define L 8 

//void dsa_kernel(data_t A[I + 1][K + 1][L + 1], data_t B[K + 1][J + 1], data_t C[L + 1][J + 1], data_t D[I + 1][J + 1]);
void dsa_kernel(data_t A[I][K][L], data_t B[K][J], data_t C[L][J], data_t D[I][J]);
