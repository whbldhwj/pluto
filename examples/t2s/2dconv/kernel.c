#include "kernel.h"

//void dsa_kernel(data_t X[R + 2][S + 2], data_t W[3][3], data_t Z[R][S]) {
//  data_t X_ext[R][S][3][3];
//  data_t W_ext[R][S][3][3];
//  data_t Z_ext[R][S][3][3];
//
//  for (int r = 0; r < R; r++)
//    for (int s = 0; s < S; s++) {
//      for (int i = 0; i < 3; i++) {
//        // for (int j = 0; j < 3; j++) {
//        for (int j = 3 - 1; j >= 0; j--) {  
//
//          // reuse at (1, -1) / (s, j) direction
//          if (s == 0) {
//            X_ext[r][s][i][j] = X[(r + 1) + i - 1][(s + 1) + j - 1];
//          } else if (j == 3 - 1 && s > 0) {
//            X_ext[r][s][i][j] = X[(r + 1) + i - 1][(s + 1) + j - 1];
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
//          if (i == 0 && j == 3 - 1) {
//            Z_ext[r][s][i][j] = X_ext[r][s][i][j] * W_ext[r][s][i][j];
//          } else {
//            if (i > 0 && j == 3 - 1) {
//              Z_ext[r][s][i][j] = Z_ext[r][s][i - 1][0] + X_ext[r][s][i][j] * W_ext[r][s][i][j];
//            } else {
//              Z_ext[r][s][i][j] = Z_ext[r][s][i][j + 1] + X_ext[r][s][i][j] * W_ext[r][s][i][j];
//            }
//          }
//        }
//      }
//      Z[r][s] = Z_ext[r][s][3 - 1][0];
//    }
//}

//void dsa_kernel(data_t X[R + 2][S + 2], data_t W[3][3], data_t Z[R][S]) {
//#pragma scop  
//  for (int r = 0; r < 32; r++)
//    for (int s = 0; s < 32; s++) {
//      for (int i = 0; i < 3; i++)
//        for (int j = 0; j < 3; j++) {
//          if (i == 0 && j == 0) 
//            Z[r][s] = 0;
//          Z[r][s] = Z[r][s] + X[r + i][s + j] * W[i][j];
//        }
//    }
//#pragma endscop  
//}

void dsa_kernel(data_t X[R + 2][S + 2], data_t W[3][3], data_t Z[R][S]) {
  data_t Z_ext[R][S][3][3];
#pragma scop  
  for (int r = 0; r < 32; r++)
    for (int s = 0; s < 32; s++) {
      for (int i = 0; i < 3; i++)
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
        }
    }
#pragma endscop  

  for (int r = 0; r < 32; r++)
    for (int s = 0; s < 32; s++) {
      Z[r][s] = Z_ext[r][s][2][2];
    }
}

//#define S1(r,s,i,j) Z[r][s] = 0;
//#define S2(r,s,i,j) Z[r][s] = Z[r][s] + X[r + i][s + j] * W[i][j];
//
//void dsa_kernel(data_t X[R + 2][S + 2], data_t W[3][3], data_t Z[R][S]) {
//int t1, t2, t3, t4, t5;
//
///* Start of CLooG code */
//for (t1=0;t1<=31;t1++) {
//  for (t2=0;t2<=2;t2++) {
//    for (t3=0;t3<=31;t3++) {
//      if (t2 == 0) {
//        S1(t3,t1,0,0);
//      }
//      for (t4=2*t2+t3;t4<=2*t2+t3+2;t4++) {
//        S2(t3,t1,t2,(-2*t2-t3+t4));
//      }
//    }
//  }
//}
///* End of CLooG code */
//}
