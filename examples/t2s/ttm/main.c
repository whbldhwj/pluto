/*
 * This code implements the Tensor Times Matrix (TTM), which performs:
 * C(i,j,k) += A(i,j,l) * B(l,k)
 * Input: A[I][J][L], B[L][K]
 * Output: C[I][J][K]
 */

#include "kernel.h"

int main(){
  // declarations
  static data_t A[I][J][L];
  static data_t B[L][K];
  static data_t C[I][J][K];
  static data_t C_dsa[I][J][K];

  // data initialization
  for (int i = 0; i < I; i++)
    for (int j = 0; j < J; j++) 
      for (int l = 0; l < L; l++) {
        A[i][j][l] = (float)rand() / RAND_MAX;
      }
  for (int l = 0; l < L; l++)
    for (int k = 0; k < K; k++) {
      B[l][k] = (float)rand() / RAND_MAX;
    }

  // computation
  for (int i = 0; i < I; i++)
    for (int j = 0; j < J; j++) 
      for (int k = 0; k < K; k++) {
        C[i][j][k] = 0;
        for (int l = 0; l < L; l++) {
          C[i][j][k] += A[i][j][l] * B[l][k];
        }
      }

  dsa_kernel(A, B, C_dsa);

  // comparison
  int err = 0;
  float thres = 0.001;
  for (int i = 0; i < I; i++) 
    for (int j = 0; j < J; j++) 
      for (int k = 0; k < K; k++) {
        if (fabs(C_dsa[i][j][k] - C[i][j][k]) > thres) {
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
