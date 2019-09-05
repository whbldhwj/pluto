#include "kernel.h"

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
