#include "kernel.h"

void dsa_kernel(data_t A[M][N], data_t Q[M][M], data_t R[M][N]) {
  data_t OA1_ext[M][N][N];
  data_t OA2_ext[M][N][N];
  data_t NA1_ext[M][N][N];
  data_t NA2_ext[M][N][N];
  data_t c_ext[M][N][N];
  data_t s_ext[M][N][N];
  data_t mag_ext[M][N][N];

  for (int k = 0; k < N; k++) {
    for (int i = 0; i < M - k - 1; i++) // row
      for (int j = k; j < N; j++) { // col
        // update local OA1, OA2, NA1, NA2
        if (k == 0) {
          if (i == 0) {
            OA1_ext[i][j][k] = A[M - 1 - i][j];
            OA2_ext[i][j][k] = A[M - 1 - i - 1][j];
          } else {
            OA1_ext[i][j][k] = NA2_ext[i - 1][j][k];
            OA2_ext[i][j][k] = A[M - 1 - i - 1][j];
          }
        } else {
          if (i == 0) {
            OA1_ext[i][j][k] = NA1_ext[i][j][k - 1];
            OA2_ext[i][j][k] = NA1_ext[i + 1][j][k - 1];
          } else {
            OA1_ext[i][j][k] = NA2_ext[i - 1][j][k];
            OA2_ext[i][j][k] = NA1_ext[i + 1][j][k - 1];
          }
        }

        // compute the Given's Rotation coeff
        if (j == k) {
          mag_ext[i][j][k] = sqrtf(OA1_ext[i][j][k] * OA1_ext[i][j][k] + OA2_ext[i][j][k] * OA2_ext[i][j][k]);
          c_ext[i][j][k] = OA2_ext[i][j][k] / mag_ext[i][j][k];
          s_ext[i][j][k] = OA1_ext[i][j][k] / mag_ext[i][j][k];          
        } else {
          c_ext[i][j][k] = c_ext[i][j - 1][k];
          s_ext[i][j][k] = s_ext[i][j - 1][k];
        }

        // update the NA1, NA2
        NA1_ext[i][j][k] = -OA2_ext[i][j][k] * s_ext[i][j][k] + OA1_ext[i][j][k] * c_ext[i][j][k];
        NA2_ext[i][j][k] = OA2_ext[i][j][k] * c_ext[i][j][k] + OA1_ext[i][j][k] * s_ext[i][j][k];    

        // output R
        if (i == M - k - 2) {
          R[k][j] = NA2_ext[i][j][k];
          if (k == N - 2) {
            R[k + 1][k + 1] = NA1_ext[i][j][k];
          }
        }
      }       
  }
}

//void dsa_kernel(data_t A[M][N], data_t Q[M][M], data_t R[M][N]) {
//  data_t OA1_ext[M][N][N];
//  data_t OA2_ext[M][N][N];
//  data_t NA1_ext[M][N][N];
//  data_t NA2_ext[M][N][N];
//  data_t c_ext[M][N][N];
//  data_t s_ext[M][N][N];
//  data_t mag_ext[M][N][N];
//
//  for (int k = 0; k < N; k++) {
//    for (int i = 0; i < M - k - 1; i++) // row
//      for (int j = k; j < N; j++) { // col
//        // update local OA1, OA2, NA1, NA2
//        if (k == 0) {
//          if (i == 0) {
//            OA1_ext[i][j][k] = A[M - 1 - i][j];
//            OA2_ext[i][j][k] = A[M - 1 - i - 1][j];
//          } else {
//            OA1_ext[i][j][k] = NA2_ext[i - 1][j][k];
//            OA2_ext[i][j][k] = A[M - 1 - i - 1][j];
//          }
//        } else {
//          if (i == 0) {
//            OA1_ext[i][j][k] = NA1_ext[i][j][k - 1];
//            OA2_ext[i][j][k] = NA1_ext[i + 1][j][k - 1];
//          } else {
//            OA1_ext[i][j][k] = NA2_ext[i - 1][j][k];
//            OA2_ext[i][j][k] = NA1_ext[i + 1][j][k - 1];
//          }
//        }
//
//        // compute the Given's Rotation coeff
//        if (j == k) {
//          mag_ext[i][j][k] = sqrtf(OA1_ext[i][j][k] * OA1_ext[i][j][k] + OA2_ext[i][j][k] * OA2_ext[i][j][k]);
//          c_ext[i][j][k] = OA2_ext[i][j][k] / mag_ext[i][j][k];
//          s_ext[i][j][k] = OA1_ext[i][j][k] / mag_ext[i][j][k];          
//        } else {
//          c_ext[i][j][k] = c_ext[i][j - 1][k];
//          s_ext[i][j][k] = s_ext[i][j - 1][k];
//        }
//
//        // update the NA1, NA2
//        NA1_ext[i][j][k] = -OA2_ext[i][j][k] * s_ext[i][j][k] + OA1_ext[i][j][k] * c_ext[i][j][k];
//        NA2_ext[i][j][k] = OA2_ext[i][j][k] * c_ext[i][j][k] + OA1_ext[i][j][k] * s_ext[i][j][k];    
//
//        // output R
//        if (i == M - k - 2) {
//          R[k][j] = NA2_ext[i][j][k];
//          if (k == N - 2) {
//            R[k + 1][k + 1] = NA1_ext[i][j][k];
//          }
//        }
//      }       
//  }
//}
