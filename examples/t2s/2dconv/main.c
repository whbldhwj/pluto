#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a>b)?a:b)

/*
 * This code implements the 2D convolution (3x3 filter window), which performs:
 * Z(r,s) += X(r+i-1,s+j-1) * W(i,j)
 * Input: X[R+2][S+2], W[3][3]
 * Output: Z[R][S]
 */

#include "kernel.h"

int main(){
  // declarations
  data_t X[R + 2][S + 2];
  data_t W[3][3];
  data_t Z[R][S];
  data_t Z_dsa[R][S];

  // data initialization
  for (int r = 0; r < R + 2; r++)
    for (int s = 0; s < S + 2; s++) {
      X[r][s] = (float)rand() / RAND_MAX;
    }

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
      W[i][j] = (float)rand() / RAND_MAX;
    }

  // computation
  for (int r = 0; r < R; r++)
    for (int s = 0; s < S; s++) {
      Z[r][s] = 0;
      for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
          Z[r][s] += X[r + i][s + j] * W[i][j];
        }
    }

  dsa_kernel(X, W, Z_dsa);

  // comparison
  int err = 0;
  float thres = 0.001;
  for (int r = 0; r < R; r++) 
    for (int s = 0; s < S; s++) {
      if (fabs(Z_dsa[r][s] - Z[r][s]) > thres) {
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