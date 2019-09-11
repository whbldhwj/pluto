#include "kernel.h"

/* DSA Form 0 */
// change parameters to constants
// avoid using +=
void dsa_kernel(data_t A[I + 1][K + 1], data_t B[K + 1][J + 1], data_t C[I + 1][J + 1]) {
#pragma scop
  for (int i = 1; i < 8 + 1; i++)
    for (int j = 1; j < 8 + 1; j++) {
      for (int k = 1; k < 8 + 1; k++) {
        if (k == 1)
          C[i][j] = 0;
        C[i][j] = C[i][j] + A[i][k] * B[k][j];
      }
    }
#pragma endscop
}

/* DSA Form 1 */
//void dsa_kernel(data_t A[I + 1][K + 1], data_t B[K + 1][J + 1], data_t C[I + 1][J + 1]) {
//  data_t C_ext[8 + 1][8 + 1][8 + 1];
//#pragma scop
//  for (int i = 1; i < 8 + 1; i ++)
//    for (int j = 1; j < 8 + 1; j++)
//      for (int k = 1; k < 8 + 1; k++) {
//        if (k == 1)
//          C_ext[i][j][k] = A[i][k] * B[k][j];
//        else
//          C_ext[i][j][k] = C_ext[i][j][k - 1] + A[i][k] * B[k][j];        
//        if (k == 8)
//          C[i][j] = C_ext[i][j][k];
//      }
//#pragma endscop  
//  
//  for (int i = 1; i < 8 + 1; i++)
//    for (int j = 1; j < 8 + 1; j++) {
//      C[i][j] = C_ext[i][j][8];
//    }
//}

/* DSA Form 2 */
//void dsa_kernel(data_t A[I][K], data_t B[K][J], data_t C[I][J]) {
//  data_t A_ext[I][J][K];
//  data_t B_ext[I][J][K];
//  data_t C_ext[I][J][K];
//
//  for (int i = 0; i < I; i++)
//    for (int j = 0; j < J; j++) {
//      for (int k = 0; k < K; k++) {
//        if (j == 0) {
//          A_ext[i][j][k] = A[i][k];
//        } else {
//          A_ext[i][j][k] = A_ext[i][j - 1][k];
//        }
//
//        if (i == 0) {
//          B_ext[i][j][k] = B[k][j];
//        } else {
//          B_ext[i][j][k] = B_ext[i - 1][j][k];
//        }
//
//        if (k == 0) {
//          C_ext[i][j][k] = A_ext[i][j][k] * B_ext[i][j][k];
//        } else {
//          C_ext[i][j][k] = C_ext[i][j][k - 1]  + A_ext[i][j][k] * B_ext[i][j][k];
//        }
//      }
//      C[i][j] = C_ext[i][j][K - 1];
//    }
//}

/* Constant version */
//void dsa_kernel(data_t A[I][K], data_t B[K][J], data_t C[I][J]) {
//  data_t A_ext[I][J][K];
//  data_t B_ext[I][J][K];
//  data_t C_ext[I][J][K];
//
//#pragma scop
//  for (int i = 0; i < 64; i++)
//    for (int j = 0; j < 64; j++) {
//      for (int k = 0; k < 64; k++) {
//        if (j == 0) {
//          A_ext[i][j][k] = A[i][k];
//        } else {
//          A_ext[i][j][k] = A_ext[i][j - 1][k];
//        }
//
//        if (i == 0) {
//          B_ext[i][j][k] = B[k][j];
//        } else {
//          B_ext[i][j][k] = B_ext[i - 1][j][k];
//        }
//
//        if (k == 0) {
//          C_ext[i][j][k] = A_ext[i][j][k] * B_ext[i][j][k];
//        } else {
//          C_ext[i][j][k] = C_ext[i][j][k - 1]  + A_ext[i][j][k] * B_ext[i][j][k];
//        }
//      }
//    }
//#pragma endscop
//
//  for (int i = 0; i < I; i++)
//    for (int j = 0; j < J; j++) {
//      C[i][j] = C_ext[i][j][K - 1];
//    } 
//}
