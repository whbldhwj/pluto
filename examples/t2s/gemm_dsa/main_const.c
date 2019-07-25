/*
 * This code implements the matrix multiplication, which performs:
 * C(i,j) += A(i,k) * B(k,j)
 * Input: A[I][K], B[K][J]
 * Output: C[I][J]
 */

#include "kernel.h"

int main(){
  // declarations
  data_t A[I][K];
  data_t B[K][J];
  data_t C[I][J];
  data_t C_dsa[I][J];

  // data initialization
  for (int i = 0; i < I; i++)
    for (int k = 0; k < K; k++) {
      A[i][k] = (float)rand() / RAND_MAX;
    }
  for (int k = 0; k < K; k++)
    for (int j = 0; j < J; j++) {
      B[k][j] = (float)rand() / RAND_MAX;
    }

  // computation
  for (int i = 0; i < 64; i++)
    for (int j = 0; j < 64; j++) {
      C[i][j] = 0;
      for (int k = 0; k < 64; k++) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }

  dsa_kernel(A, B, C_dsa);

  // comparison
  int err = 0;
  float thres = 0.001;
  for (int i = 0; i < I; i++) 
    for (int j = 0; j < J; j++) {
      if (fabs(C_dsa[i][j] - C[i][j]) > thres) {
        err++;
      }
    }

  if (err) {
    printf("Test failed with %d errors!\n", err);
    return -1;
  } else {
    printf("Test passed!\n");
    return 0;
  }
}
