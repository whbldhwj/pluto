#include "kernel.h"

/* DSA Form 0 */
// change paramters to constants
// avoid using +=
//void dsa_kernel(data_t X[R + 2][S + 2], data_t W[3][3], data_t Z[R][S]) {
//#pragma scop  
//  for (int r = 0; r < 32; r++)
//    for (int s = 0; s < 32; s++) {
//      Z[r][s] = 0;
//      for (int i = 0; i < 3; i++)
//        for (int j = 0; j < 3; j++) {
//          Z[r][s] = Z[r][s] + X[r + i][s + j] * W[i][j];
//        }
//    }
//#pragma endscop  
//}

/* DSA Form 1 */
void dsa_kernel(data_t X[R + 2][S + 2], data_t W[3][3], data_t Z[R][S]) {
  data_t Z_ext[R][S][3][3];
#pragma scop  
  for (int r = 0; r < 32; r++)
    for (int s = 0; s < 32; s++) {
      for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
          if (i == 0 && j == 0) 
            Z_ext[r][s][i][j] = X[r + i][s + j] * W[i][j];
          else if (i > 0 && j == 0) {
            Z_ext[r][s][i][j] = Z_ext[r][s][i - 1][2] + X[r + i][s + j] * W[i][j];
          } else if (i > 0 && j > 0) {
            Z_ext[r][s][i][j] = Z_ext[r][s][i][j - 1] + X[r + i][s + j] * W[i][j];
          } else if (i == 0 && j > 0) {                   
            Z_ext[r][s][i][j] = Z_ext[r][s][i][j - 1] + X[r + i][s + j] * W[i][j];
          }

          if (i == 2 && j == 2) {
            Z[r][s] = Z_ext[r][s][2][2];
          }
        }
      }
    }
#pragma endscop  

  for (int r = 0; r < 32; r++)
    for (int s = 0; s < 32; s++) {
      Z[r][s] = Z_ext[r][s][2][2];
    }
}

/* DSA From 2 */
//void dsa_kernel(data_t X[R + 2][S + 2], data_t W[3][3], data_t Z[R][S]) {
//  data_t X_ext[R][S][3][3];
//  data_t W_ext[R][S][3][3];
//  data_t Z_ext[R][S][3][3];
//
//#pragma scop
//  for (int r = 0; r < R; r++)
//    for (int s = 0; s < S; s++) {
//      for (int i = 0; i < 3; i++) {
//        for (int j = 0; j < 3; j++) {
//          // reuse at (1, -1) / (s, j) direction
//          if (s == 0) {
//            X_ext[r][s][i][j] = X[r + i][s + j];
//          } else if (j == 3 - 1 && s > 0) {
//            X_ext[r][s][i][j] = X[r + i][s + j];
//          } else {
//            X_ext[r][s][i][j] = X_ext[r][s - 1][i][j + 1];
//          }
//
//          // reuse at r-axis
//          if (r == 0) {
//            if (s == 0) {
//              W_ext[r][s][i][j] = W[i][j];
//            } else {
//              W_ext[r][s][i][j] = W_ext[r][s - 1][i][j];
//            }
//          } else {
//            W_ext[r][s][i][j] = W_ext[r - 1][s][i][j];
//          }
//
//          if (i == 0 && j == 0) {
//            Z_ext[r][s][i][j] = X_ext[r][s][i][j] * W_ext[r][s][i][j];
//          } else {
//            if (i > 0 && j == 0) {
//              Z_ext[r][s][i][j] = Z_ext[r][s][i - 1][3 - 1] + X_ext[r][s][i][j] * W_ext[r][s][i][j];
//            } else {
//              Z_ext[r][s][i][j] = Z_ext[r][s][i][j - 1] + X_ext[r][s][i][j] * W_ext[r][s][i][j];
//            }
//          }
//        }
//      }
//    }
//#pragma endscop
//
//  for (int r = 0; r < R; r++)
//    for (int s = 0; s < S; s++) {
//      Z[r][s] = Z_ext[r][s][3 - 1][3 - 1];
//    }
//}
