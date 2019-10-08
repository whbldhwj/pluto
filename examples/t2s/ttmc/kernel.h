#include "stdio.h"
#include "stdlib.h"
#include "math.h"

typedef float data_t;
#define I 8 
#define J 8 
#define K 8 
#define L 8 
#define M 8 

//void dsa_kernel(data_t A[I][L][M], data_t B[L][J], data_t C[M][K], data_t D[I][J][K]);
void dsa_kernel(data_t A[I + 1][L + 1][M + 1], data_t B[L + 1][J + 1], data_t C[M + 1][K + 1], data_t D[I + 1][J + 1][K + 1]);
