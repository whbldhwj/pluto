#include "kernel.h"

void dsa_kernel(data_t X[R + 2][S + 2], data_t W[3][3], data_t Z[R][S]) {
  data_t X_ext[R][S][3][3];
  data_t W_ext[R][S][3][3];
  data_t Z_ext[R][S][3][3];

  for (int r = 0; r < R; r++)
    for (int s = 0; s < S; s++) {
      for (int i = 0; i < 3; i++) {
        // for (int j = 0; j < 3; j++) {
        for (int j = 3 - 1; j >= 0; j--) {  

          // reuse at (1, -1) / (s, j) direction
          if (s == 0) {
            X_ext[r][s][i][j] = X[(r + 1) + i - 1][(s + 1) + j - 1];
          } else if (j == 3 - 1 && s > 0) {
            X_ext[r][s][i][j] = X[(r + 1) + i - 1][(s + 1) + j - 1];
          } else {
            X_ext[r][s][i][j] = X_ext[r][s - 1][i][j + 1];
          }

          // reuse at r-axis
          if (r == 0) {
            if (s == 0) {
              W_ext[r][s][i][j] = W[i][j];
            } else {
              W_ext[r][s][i][j] = W_ext[r][s - 1][i][j];
            }
          } else {
            W_ext[r][s][i][j] = W_ext[r - 1][s][i][j];
          }

          if (i == 0 && j == 3 - 1) {
            Z_ext[r][s][i][j] = X_ext[r][s][i][j] * W_ext[r][s][i][j];
          } else {
            if (i > 0 && j == 3 - 1) {
              Z_ext[r][s][i][j] = Z_ext[r][s][i - 1][0] + X_ext[r][s][i][j] * W_ext[r][s][i][j];
            } else {
              Z_ext[r][s][i][j] = Z_ext[r][s][i][j + 1] + X_ext[r][s][i][j] * W_ext[r][s][i][j];
            }
          }
        }
      }
      Z[r][s] = Z_ext[r][s][3 - 1][0];
    }
}

//void dsa_kernel(data_t X[R + 2][S + 2], data_t W[3][3], data_t Z[R][S]) {
//  data_t X_ext[R + 2][S + 2][3][3];
//  data_t W_ext[R + 2][S + 2][3][3];
//  data_t Z_ext[R + 2][S + 2][3][3];
//
//  for (int r = 0; r < R + 2; r++)
//    for (int s = 0; s < S + 2; s++) {
//      for (int i = 0; i < 3; i++) {
//        for (int j = 0; j < 3; j++) {
//          // reuse at i-axis
//          if (i == 0) {
//            if (j == 0) {
//              X_ext[r][s][i][j] = X[r][s];
//            } else {
//              X_ext[r][s][i][j] = X_ext[r][s][i][j - 1];
//            }
//          } else {
//            X_ext[r][s][i][j] = X_ext[r][s][i - 1][j];
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
//        }
//      }
//    }
//
//  for (int r = 0; r < R + 2; r++)
//    for (int s = 0; s < S + 2; s++) {
//      for (int i = 0; i < 3; i++) {
//        for (int j = 0; j < 3; j++) {
//          if (r > 0 && r < R + 1 && s > 0 && s < S + 1) {
//            if (i == 0 && j == 0) {
//              Z_ext[r][s][i][j] = X_ext[r + i - 1][s + j - 1][i][j] * W_ext[r][s][i][j];
//            } else {
//              if (i > 0 && j == 0) {
//                Z_ext[r][s][i][j] = Z_ext[r][s][i - 1][3 - 1] + X_ext[r + i - 1][s + j - 1][i][j] * W_ext[r][s][i][j];
//              } else {
//                Z_ext[r][s][i][j] = Z_ext[r][s][i][j - 1] + X_ext[r + i - 1][s + j - 1][i][j] * W_ext[r][s][i][j]; 
//              }
//            }
//          }
//        }
//      }
//      if (r > 0 && r < R + 1 && s > 0 && s < S + 1) {
//        Z[r - 1][s - 1] = Z_ext[r][s][3 - 1][3 - 1];
//      }
//    }
//}
