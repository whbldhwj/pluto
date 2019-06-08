/**
  * This file is automatically gneerated by PolySA CodeGen.
  * Author: Jie Wang
  */

#include "common_header_U1.h"

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

  // hardware implmenetation
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

  // hardware kernel
  top_kernel(A, B, C);

  // software implementation
  data_t0 A_sw[I][K];
  data_t1 B_sw[J][K];
  data_t2 C_sw[I][J];
  
  // initialization
  for (i = 0; i < I; i++)
    for (k = 0; k < K; k++)
      A_sw[i][k] = i * K + k;
  for (j = 0; j < J; j++)
    for (k = 0; k < K; k++)
      B_sw[j][k] = j * K + k;
  for (i = 0; i < I; i++)
    for (j = 0; j < J; j++)
      C_sw[i][j] = 0;

  for (i = 0 ; i < I; i++)
    for (j = 0; j < J; j++)
      for (k = 0; k < K; k++)
        C_sw[i][j] = C_sw[i][j] + A_sw[i][k] * B_sw[j][k];

  // compare the results
  int err = 0;
  for (i = 0; i < I; i++)
    for (j = 0; j < J; j++) {
      if (abs(C_sw[i][j] - C[i][j]) > 0.01)
        err++;      
    }

  if (err == 0) {
    printf("test passed!\n");
    return 0;
  } else {
    printf("test failed! %d errors!\n", err);
    return 1;
  }
}
