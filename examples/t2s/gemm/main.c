/*
 * This code implements the matrix multiplication, which performs:
 * C(i,j) += A(i,k) * B(k,j)
 * Input: A[I][K], B[K][J]
 * Output: C[I][J]
 */

#include "kernel.h"

int main(){
  // declarations
  data_t A[I + 1][K + 1];
  data_t B[K + 1][J + 1];
  data_t C[I + 1][J + 1];
  data_t C_dsa[I + 1][J + 1];

  // data initialization
  for (int i = 0; i < I + 1; i++)
    for (int k = 0; k < K + 1; k++) {
      A[i][k] = (float)rand() / RAND_MAX;
    }
  for (int k = 0; k < K + 1; k++)
    for (int j = 0; j < J + 1; j++) {
      B[k][j] = (float)rand() / RAND_MAX;
    }

  for (int i = 0; i < I + 1; i++)
    for (int j = 0; j < J + 1; j++)
      C[i][j] = 0;

  // computation
  for (int i = 1; i <= I; i++)
    for (int j = 1; j <= J; j++) {
//      C[i][j] = 0;
      for (int k = 1; k <= K; k++) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }

  dsa_kernel(A, B, C_dsa);

  // comparison
  int err = 0;
  float thres = 0.001;
  for (int i = 1; i <= I; i++) 
    for (int j = 1; j <= J; j++) {
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
