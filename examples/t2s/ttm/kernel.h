#include "stdio.h"
#include "stdlib.h"
#include "math.h"

typedef float data_t;
#define I 8 
#define J 8 
#define K 8 
#define L 8 

//void dsa_kernel(data_t A[I][J][L], data_t B[L][K], data_t C[I][J][K]);
void dsa_kernel(data_t A[I + 1][J + 1][L + 1], data_t B[L + 1][K + 1], data_t C[I + 1][J + 1][K + 1]);
