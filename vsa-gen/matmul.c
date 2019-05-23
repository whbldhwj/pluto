#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// global parameters - start
#define I 64
#define J 64
#define K 64
// global parameters - end

// type definition - start
typedef float data_t0;
typedef float data_t1;
typedef float data_t2;
// type definition - end

int main(){
  int i, j, k;

  // buffers - start
  data_t0 A[I][K];
  data_t1 B[J][K];
  data_t2 C[I][J];
  // buffers - end
  
  // initialization
  for (i = 0; i < I; i++)
    for (k = 0; k < K; k++)
      A[i][k] = i * K + k;
  for (j = 0; j < J; j++)
    for (k = 0; k < K; k++)
      B[j][k] = j * K + k;
  for (i = 0; i < I; i++)
    for (j = 0; j < J; j++)
      C[i][j] = 0;

#pragma scop
  for (i = 0 ; i < I; i++)
    for (j = 0; j < J; j++)
      for (k = 0; k < K; k++)
        C[i][j] = C[i][j] + A[i][k] * B[j][k];
#pragma endscop  

  return 0;
}
