#include "kernel.h"

void dsa_kernel(data_t A[I][K][L], data_t B[K][J], data_t C[L][J], data_t D[I][J]){
#pragma scop
  for (int i = 0; i < I; i++)
    for (int j = 0; j < J; j++)
      for (int k = 0; k < K; k++)
        for (int l = 0; l < L; l++) {
          if (k == 0 && l == 0) {
            D[i][j] = 0;
          }
          D[i][j] = D[i][j] + A[i][k][l] * B[k][j] * C[l][j];
        }
#pragma endscop
}

//void dsa_kernel(data_t A[I + 1][K + 1][L + 1], data_t B[K + 1][J + 1], data_t C[L + 1][J + 1], data_t D[I + 1][J + 1]){
//#pragma scop
//  for (int i = 1; i <= 8; i++)
//    for (int j = 1; j <= 8; j++)
//      for (int k = 1; k <= 8; k++)
//        for (int l = 1; l <= 8; l++) {
//          if (k == 1 && l == 1) {
//            D[i][j] = 0;
//          }
//          D[i][j] = D[i][j] + A[i][k][l] * B[k][j] * C[l][j];
//        }
//#pragma endscop
//}

//void dsa_kernel(data_t A[I][K][L], data_t B[K][J], data_t C[L][J], data_t D[I][J]) {
//  static data_t B_CC1_E[64][64][64][64];
//  static data_t C_CC2_E[64][64][64][64];
//  static data_t A_CC3_E[64][64][64][64];
//  static data_t D_CC0_I[64][64][64][64];
//  static data_t APP[64][64][64][64];
//
//  for (int t1 = 0; t1 <= 63; t1++)
//    for (int t2 = 0; t2 <= 63; t2++)
//      for (int t3 = 0; t3 <= 63; t3++)
//        for (int t4 = 0; t4 <= 63; t4++) {
//          if (t2 == 0) {
//            B_CC1_E[t1][t2][t3][t4] = B[t1][t3];
//          } else if (t2 >= 1) {
//            B_CC1_E[t1][t2][t3][t4] = B_CC1_E[t1][t2 - 1][t3][t4];
//          }
//
//          if (t1 == 0) {
//            C_CC2_E[t1][t2][t3][t4] = C[t4][t3];
//          } else if (t1 >= 1) {
//            C_CC2_E[t1][t2][t3][t4] = C_CC2_E[t1 - 1][t2][t3][t4];
//          }
//
//          if (t3 == 0) {
//            A_CC3_E[t1][t2][t3][t4] = A[t2][t1][t4];
//          } else if (t3 >= 1) {
//            A_CC3_E[t1][t2][t3][t4] = A_CC3_E[t1][t2][t3 - 1][t4];
//          }
//
//          if (t4 == 0 && t1 == 0) {
//            D_CC0_I[t1][t2][t3][t4] = 0;
//          } 
//          if (t4 - 1 >= 0) {
//            D_CC0_I[t1][t2][t3][t4] = D_CC0_I[t1][t2][t3][t4 - 1] + A_CC3_E[t1][t2][t3][t4] * B_CC1_E[t1][t2][t3][t4] * C_CC2_E[t1][t2][t3][t4];
//          } 
//          if (t4 == 0 && t1 - 1 >= 0) {
//            D_CC0_I[t1][t2][t3][t4] = D_CC0_I[t1 - 1][t2][t3][t4 + 63] + A_CC3_E[t1][t2][t3][t4] * B_CC1_E[t1][t2][t3][t4] * C_CC2_E[t1][t2][t3][t4]; 
//          }
//          if (t4 == 0 && t1 == 0) {
//            D_CC0_I[t1][t2][t3][t4] = D_CC0_I[t1][t2][t3][t4] +  A_CC3_E[t1][t2][t3][t4] * B_CC1_E[t1][t2][t3][t4] * C_CC2_E[t1][t2][t3][t4];
//          }
//
//          if (t4 - 63 == 0 && t1 - 63 == 0) {
//            APP[t1][t2][t3][t4] = D_CC0_I[t1][t2][t3][t4];
//          }
//        }
//
//  for (int i = 0; i < I; i++)
//    for (int j = 0; j < J; j++) {
//      D[i][j] = APP[63][i][j][63];
//    }
//}

//void dsa_kernel(data_t A[I][K][L], data_t B[K][J], data_t C[L][J], data_t D[I][J]){
//  static data_t A_ext[I][J][K][L];
//  static data_t B_ext[I][J][K][L];
//  static data_t C_ext[I][J][K][L];
//  static data_t D_ext[I][J][K][L];
//
//  for (int i = 0; i < I; i++) {
//    for (int j = 0; j < J; j++) {
//      for (int k = 0; k < K; k++) {
//        for (int l = 0; l < L; l++) {
//          if (j == 0) {
//            A_ext[i][j][k][l] = A[i][k][l];
//          } else {
//            A_ext[i][j][k][l] = A_ext[i][j - 1][k][l];
//          }
//          
//          // reuse at i-axis
//          if (i == 0) {
//            if (l == 0) {
//              B_ext[i][j][k][l] = B[k][j];
//            } else {
//              // initial distribution at l-axis
//              B_ext[i][j][k][l] = B_ext[i][j][k][l - 1];
//            }
//          } else {
//            B_ext[i][j][k][l] = B_ext[i - 1][j][k][l];
//          }
//
//          // reuse at i-axis
//          if (i == 0) {
//            if (k == 0) {
//              C_ext[i][j][k][l] = C[l][j];
//            } else {
//              // initial distribution at k-axis
//              C_ext[i][j][k][l] = C_ext[i][j][k - 1][l];
//            }
//          } else {
//            C_ext[i][j][k][l] = C_ext[i - 1][j][k][l];
//          }
//
//
//          if (k == 0 && l == 0) {
//            D_ext[i][j][k][l] = A_ext[i][j][k][l] * B_ext[i][j][k][l] * C_ext[i][j][k][l];
//          } else if (k > 0 && l == 0){
//            D_ext[i][j][k][l] = D_ext[i][j][k - 1][L - 1] + A_ext[i][j][k][l] * B_ext[i][j][k][l] * C_ext[i][j][k][l];
//          } else {
//            D_ext[i][j][k][l] = D_ext[i][j][k][l - 1] + A_ext[i][j][k][l] * B_ext[i][j][k][l] * C_ext[i][j][k][l];
//          }
//        }      
//      }
//      D[i][j] = D_ext[i][j][K - 1][L - 1];
//    }
//  }
//}
