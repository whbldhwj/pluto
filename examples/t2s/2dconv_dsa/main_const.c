#define S1(r,s,i,j) Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];

/*
 * This code implements the 2D convolution (3x3 filter window), which performs:
 * Z(r,s) += X(r+i-1,s+j-1) * W(i,j)
 * Input: X[R+2][S+2], W[3][3]
 * Output: Z[R][S]
 */

#include "kernel.h"

int main(){
  // declarations
  data_t X[32 + 2][32 + 2];
  data_t W[3][3];
  data_t Z[32][32];
  data_t Z_dsa[32][32];

  // data initialization
  for (int r = 0; r < 32 + 2; r++)
    for (int s = 0; s < 32 + 2; s++) {
      X[r][s] = (float)rand() / RAND_MAX;
    }

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
      W[i][j] = (float)rand() / RAND_MAX;
    }

  for (int r = 0; r < 32; r++)
    for (int s = 0; s < 32; s++) {
      Z[r][s] = 0;
    }

  // computation
#pragma scop
  for (int r = 1; r < 32 + 1; r++)
    for (int s = 1 ; s < 32 + 1; s++) {
//      Z[r - 1][s - 1] = 0;
      for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
          if (i == 0 && j == 0)
            Z[r - 1][s - 1] = 0;
          Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];
        }
    }
#pragma endscop  

  dsa_kernel(X, W, Z_dsa);

  // comparison
  int err = 0;
  float thres = 0.001;
  for (int r = 0; r < 32; r++) 
    for (int s = 0; s < 32; s++) {
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
