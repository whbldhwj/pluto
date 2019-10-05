#include "kernel.h"

/* DSA Form 0 */
// change paramters to constants
// avoid using +=
void dsa_kernel(data_t X[R + 3 + 1][S + 3 + 1], data_t W[3 + 1][3 + 1], data_t Z[R + 1][S + 1]) {
#pragma scop  
  for (int r = 1; r < 8 + 1; r++)
    for (int s = 1; s < 8 + 1; s++) {
      for (int i = 1; i < 3 + 1; i++)
        for (int j = 1; j < 3 + 1; j++) {
          if (i == 1 && j == 1) 
            Z[r][s] = 0;
          Z[r][s] = Z[r][s] + X[r + i][s + j] * W[i][j];
        }
    }
#pragma endscop  
}

/* DSA Form 1 */
//void dsa_kernel(data_t X[R + 3 + 1][S + 3 + 1], data_t W[3 + 1][3 + 1], data_t Z[R + 1][S + 1]) {
//  data_t Z_ext[R + 1][S + 1][3 + 1][3 + 1];
//#pragma scop  
//  for (int r = 1; r < 8 + 1; r++)
//    for (int s = 1; s < 8 + 1; s++) {
//      for (int i = 1; i < 3 + 1; i++) {
//        for (int j = 1; j < 3 + 1; j++) {
//          if (i == 1 && j == 1) 
//            Z_ext[r][s][i][j] = X[r + i][s + j] * W[i][j];
//          else if (i > 1 && j == 1) {
//            Z_ext[r][s][i][j] = Z_ext[r][s][i - 1][3] + X[r + i][s + j] * W[i][j];
//          } else if (i > 1 && j > 1) {
//            Z_ext[r][s][i][j] = Z_ext[r][s][i][j - 1] + X[r + i][s + j] * W[i][j];
//          } else if (i == 1 && j > 1) {                   
//            Z_ext[r][s][i][j] = Z_ext[r][s][i][j - 1] + X[r + i][s + j] * W[i][j];
//          }
//
//          if (i == 3 && j == 3) {
//            Z[r][s] = Z_ext[r][s][3][3];
//          }
//        }
//      }
//    }
//#pragma endscop  
//
//  for (int r = 1; r < 8 + 1; r++)
//    for (int s = 1; s < 8 + 1; s++) {
//      Z[r][s] = Z_ext[r][s][3][3];
//    }
//}
