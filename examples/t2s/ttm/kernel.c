#include "kernel.h"

//void dsa_kernel(data_t A[I][J][L], data_t B[L][K], data_t C[I][J][K]){
//#pragma scop
//  for (int i = 0; i < I; i++)
//    for (int j = 0; j < J; j++)
//      for (int k = 0; k < K; k++) 
//        for (int l = 0; l < L; l++) {
//          if (l == 0) 
//            C[i][j][k] = 0;        
//          C[i][j][k] = C[i][j][k] + A[i][j][l] * B[l][k];
//        }
//#pragma endscop
//}

void dsa_kernel(data_t A[I + 1][J + 1][L + 1], data_t B[L + 1][K + 1], data_t C[I + 1][J + 1][K + 1]){
#pragma scop
  for (int i = 1; i <= I; i++)
    for (int j = 1; j <= J; j++)
      for (int k = 1; k <= K; k++) 
        for (int l = 1; l <= L; l++) {
          if (l == 1) 
            C[i][j][k] = 0;        
          C[i][j][k] = C[i][j][k] + A[i][j][l] * B[l][k];
        }
#pragma endscop
}

//void dsa_kernel(data_t A[I][J][L], data_t B[L][K], data_t C[I][J][K]){
//  static data_t A_ext[I][J][K][L];
//  static data_t B_ext[I][J][K][L];
//  static data_t C_ext[I][J][K][L];
//
//  for (int i = 0; i < I; i++) {
//    for (int j = 0; j < J; j++) {
//      for (int k = 0; k < K; k++) {
//        for (int l = 0; l < L; l++) {
//          if (k == 0) {
//            A_ext[i][j][k][l] = A[i][j][l];
//          } else {
//            A_ext[i][j][k][l] = A_ext[i][j][k - 1][l];
//          }
//          
//          // reuse at i-axis
//          if (i == 0) {
//            if (j == 0) {
//              B_ext[i][j][k][l] = B[l][k];
//            } else {
//              // initial distribution at j-axis
//              B_ext[i][j][k][l] = B_ext[i][j - 1][k][l];
//            }
//          } else {
//            B_ext[i][j][k][l] = B_ext[i - 1][j][k][l];
//          }
//
//          if (l == 0) {
//            C_ext[i][j][k][l] = A_ext[i][j][k][l] * B_ext[i][j][k][l];
//          } else {            
//            C_ext[i][j][k][l] = C_ext[i][j][k][l - 1] + A_ext[i][j][k][l] * B_ext[i][j][k][l];
//          }
//        }     
//        C[i][j][k] = C_ext[i][j][k][L - 1];
//      }
//    }
//  }
//}
