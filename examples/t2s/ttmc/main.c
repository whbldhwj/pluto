/*
 * This code implements the Chain of Tensor-matrix multiplications (TTMc), which performs:
 * D(i,j,k) += A(i,l,m) * B(l,j) * C(m,k)
 * Input: A[I][L][M], B[L][J], C[M][K]
 * Output: D[I][J][K]
 */

#include "kernel.h"

int main(){
  // declarations
  static data_t A[I][L][M];
  static data_t B[L][J];
  static data_t C[M][K];
  static data_t D[I][J][K];
  static data_t D_dsa[I][J][K];

  // data initialization
  for (int i = 0; i < I; i++)
    for (int l = 0; l < L; l++) 
      for (int m = 0; m < M; m++) {
        A[i][l][m] = (float)rand() / RAND_MAX;
      }
  for (int l = 0; l < L; l++)
    for (int j = 0; j < J; j++) {
      B[l][j] = (float)rand() / RAND_MAX;
    }
  for (int m = 0; m < M; m++)
    for (int k = 0; k < K; k++) {
      C[m][k] = (float)rand() / RAND_MAX;
    }
  
  // computation
  for (int i = 0; i < I; i++)
    for (int j = 0; j < J; j++) 
      for (int k = 0; k < K; k++) {
        D[i][j][k] = 0;        
        for (int l = 0; l < L; l++) 
          for (int m = 0; m < M; m++) {
            D[i][j][k] += A[i][l][m] * B[l][j] * C[m][k];
          }
      }    

  dsa_kernel(A, B, C, D_dsa);

  // comparison
  int err = 0;
  float thres = 0.001;
  for (int i = 0; i < I; i++) 
    for (int j = 0; j < J; j++) 
      for (int k = 0; k < K; k++) {
        if (fabs(D_dsa[i][j][k] - D[i][j][k]) > thres) {
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
