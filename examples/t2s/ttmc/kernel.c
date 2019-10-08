#include "kernel.h"

//void dsa_kernel(data_t A[I][L][M], data_t B[L][J], data_t C[M][K], data_t D[I][J][K]){
//#pragma scop  
//  for (int i = 0; i < I; i++)
//    for (int j = 0; j < J; j++)
//      for (int k = 0; k < K; k++) {
//        D[i][j][k] = 0;
//        for (int l = 0; l < L; l++)
//          for (int m = 0; m < M; m++) {
//            D[i][j][k] = D[i][j][k] + A[i][l][m] * B[l][j] * C[m][k];
//          }
//      }
//#pragma endscop
//}

void dsa_kernel(data_t A[I + 1][L + 1][M + 1], data_t B[L + 1][J + 1], data_t C[M + 1][K + 1], data_t D[I + 1][J + 1][K + 1]){
#pragma scop  
  for (int i = 1; i <= I; i++)
    for (int j = 1; j <= J; j++)
      for (int k = 1; k <= K; k++) {
        D[i][j][k] = 0;
        for (int l = 1; l <= L; l++)
          for (int m = 1; m <= M; m++) {
            D[i][j][k] = D[i][j][k] + A[i][l][m] * B[l][j] * C[m][k];
          }
      }
#pragma endscop
}

//void dsa_kernel(data_t A[I][L][M], data_t B[L][J], data_t C[M][K], data_t D[I][J][K]){
//  static data_t A_ext[I][J][K][L][M];
//  static data_t B_ext[I][J][K][L][M];
//  static data_t C_ext[I][J][K][L][M];
//  static data_t D_ext[I][J][K][L][M];
//
//  for (int i = 0; i < I; i++) {
//    for (int j = 0; j < J; j++) {
//      for (int k = 0; k < K; k++) {
//        for (int l = 0; l < L; l++) {
//          for (int m = 0; m < M; m++) {
//            // reuse at j-axis
//            if (j == 0) {
//              if (k == 0) {
//                A_ext[i][j][k][l][m] = A[i][l][m];
//              } else {
//                A_ext[i][j][k][l][m] = A_ext[i][j][k - 1][l][m];
//              }
//            } else {
//              A_ext[i][j][k][l][m] = A_ext[i][j - 1][k][l][m];
//            }
//            
//            // reuse at i-axis
//            if (i == 0) {
//              if (k == 0) {
//                if (m == 0) {
//                  B_ext[i][j][k][l][m] = B[l][j];
//                } else {
//                  B_ext[i][j][k][l][m] = B_ext[i][j][k][l][m - 1];
//                }
//              } else {
//                B_ext[i][j][k][l][m] = B_ext[i][j][k - 1][l][m];
//              }
//            } else {
//              B_ext[i][j][k][l][m] = B_ext[i - 1][j][k][l][m];
//            }
//
//            // reuse at i-axis
//            if (i == 0) {
//              if (j == 0) {
//                if (l == 0) {
//                  C_ext[i][j][k][l][m] = C[m][k];
//                } else {
//                  C_ext[i][j][k][l][m] = C_ext[i][j][k][l - 1][m];
//                }
//              } else {
//                C_ext[i][j][k][l][m] = C_ext[i][j - 1][k][l][m];
//              }
//            } else {
//              C_ext[i][j][k][l][m] = C_ext[i - 1][j][k][l][m];
//            }
//
//            if (l == 0 && m == 0) {
//              D_ext[i][j][k][l][m] = A_ext[i][j][k][l][m] * B_ext[i][j][k][l][m] * C_ext[i][j][k][l][m];
//            } else if (l > 0 && m == 0) {
//              D_ext[i][j][k][l][m] = D_ext[i][j][k][l - 1][M - 1] + A_ext[i][j][k][l][m] * B_ext[i][j][k][l][m] * C_ext[i][j][k][l][m];              
//            } else {
//              D_ext[i][j][k][l][m] = D_ext[i][j][k][l][m - 1] + A_ext[i][j][k][l][m] * B_ext[i][j][k][l][m] * C_ext[i][j][k][l][m];              
//            }
//          }
//        }     
//        D[i][j][k] = D_ext[i][j][k][L - 1][M - 1];
//      }
//    }
//  }
//}
