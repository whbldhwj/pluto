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
  data_t X[R + 3 + 1][S + 3 + 1];
  data_t W[3 + 1][3 + 1];
  data_t Z[R + 1][S + 1];
  data_t Z_dsa[R + 1][S + 1];

  // data initialization
  for (int r = 0; r < R + 3 + 1; r++)
    for (int s = 0; s < S + 3 + 1; s++) {
      X[r][s] = r;
    }

  for (int i = 0; i < 3 + 1; i++)
    for (int j = 0; j < 3 + 1; j++) {
      W[i][j] = i;
    }

  for (int r = 1; r < R + 1; r++)
    for (int s = 1; s < S + 1; s++) {
      Z[r][s] = 0;
    }

  // computation
#pragma scop
  for (int r = 1; r < R + 1; r++)
    for (int s = 1; s < S + 1; s++) {
      Z[r][s] = 0;
      for (int i = 1; i < 3 + 1; i++)
        for (int j = 1; j < 3 + 1; j++) {
          Z[r][s] += X[r + i][s + j] * W[i][j];
        }
    }
#pragma endscop  

  dsa_kernel(X, W, Z_dsa);

  // comparison
  int err = 0;
  float thres = 0.001;
  for (int r = 1; r < R + 1; r++) {
    for (int s = 1; s < S + 1; s++) {
      if (fabs(Z_dsa[r][s] - Z[r][s]) > thres) {
        err++;
      }     
      printf("%d(%d) ", Z_dsa[r][s], Z[r][s]);
    }
    printf("\n");
  }
  printf("\n");

  if (err) {
    printf("Test failed with %d errors!\n", err);
    return -1;
  } else {
    printf("Test passed!\n");
    return 0;
  }
}
