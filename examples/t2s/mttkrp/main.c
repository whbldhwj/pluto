/*
 * This code implements the Matricized Tensor Times Khatri-Rao Product (MTTKRP), which performs:
 * D(i,j) += A(i,k,l) * B(k,j) * C(l,j)
 * Input: A[I][K][L], B[K][J], C[L][J]
 * Output: D[I][J]
 */

#include "kernel.h"

int main(){
  // declarations
  static data_t A[I + 1][K + 1][L + 1];
  static data_t B[K + 1][J + 1];
  static data_t C[L + 1][J + 1];
  static data_t D[I + 1][J + 1];
  static data_t D_dsa[I + 1][J + 1];

  // data initialization
  for (int i = 0; i < I + 1; i++)
    for (int k = 0; k < K + 1; k++) 
      for (int l = 0; l < L + 1; l++) {
        A[i][k][l] = (float)rand() / RAND_MAX;
      }
  for (int k = 0; k < K + 1; k++)
    for (int j = 0; j < J + 1; j++) {
      B[k][j] = (float)rand() / RAND_MAX;
    }
  for (int l = 0; l < L + 1; l++)
    for (int j = 0; j < J + 1; j++) {
      C[l][j] = (float)rand() / RAND_MAX;
    }

  // computation
  for (int i = 1; i <= I; i++)
    for (int j = 1; j <= J; j++) {
      D[i][j] = 0;
      for (int k = 1; k <= K; k++)
        for (int l = 1; l <= L; l++) {
          D[i][j] += A[i][k][l] * B[k][j] * C[l][j];
        }
    }

  dsa_kernel(A, B, C, D_dsa);

  // comparison
  int err = 0;
  float thres = 0.001;
  for (int i = 1; i <= I; i++) 
    for (int j = 1; j <= J; j++) {
      if (fabs(D_dsa[i][j] - D[i][j]) > thres) {
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
