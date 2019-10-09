/*
 * This code implements the calculation of variance-covariance matrix.
 * The detailed algorithm can be found at:
 * https://stattrek.com/matrix-algebra/covariance-matrix.aspx
 * d = X - 11'X(1/n)
 * V = d'd(1/n)
 * Input: X[N][K]
 * Output: V[K][K]
 */

#include "kernel.h"

int main(){
  // declarations
  data_t X[N][K];
  data_t d[N][K];
  data_t V[K][K];
  data_t V_dsa[K][K];

  // data initialization
  for (int n = 0; n < N; n++)
    for (int k = 0; k < K; k++) {
      X[n][k] = (float)rand() / RAND_MAX;
    }

  // computation
  for (int n = 0; n < N; n++)
    for (int k = 0; k < K; k++) {
      data_t mean = 0;
      for (int p = 0; p < N; p++) {
        mean += X[p][k];
      }
      mean = mean / N;
      d[n][k] = X[n][k] - mean;
    }

//  for (int n = 0; n < N; n++)
//    for (int k = 0; k < K; k++) {
//      d[n][k] = X[n][k];
//      for (int p = 0; p < N; p++) {
//        d[n][k] -= X[p][k] / N;
//      }
//    }

  for (int i = 0; i < K; i++)
    for (int j = 0; j < K; j++){
      V[i][j] = 0;
      for (int k = 0; k < N; k++) {
        V[i][j] += d[k][i] * d[k][j] / N;
      }
    }

  // Further optimization: V is symmetric, and we should only
  // calculate half of it.

  dsa_kernel(d, V_dsa);

  // comparison
  int err = 0;
  float thres = 0.001;
  for (int i = 0; i < K; i++) 
    for (int j = 0; j < K; j++) {
      if (fabs(V_dsa[i][j] - V[i][j]) > thres) {
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
