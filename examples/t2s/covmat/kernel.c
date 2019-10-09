#include "kernel.h"

//void dsa_kernel(data_t d[N][K], data_t V[K][K]) {
//#pragma scop
//  for (int i = 0; i < K; i++)
//    for (int j = 0; j < K; j++) {
//      V[i][j] = 0;
//      for (int k = 0; k < N; k++) {
//        V[i][j] = V[i][j] + d[k][i] * d[k][j] / N;
//      }
//    }
//#pragma endscop
//}

void dsa_kernel(data_t d[N + 1][K + 1], data_t V[K + 1][K + 1]) {
#pragma scop
  for (int i = 1; i <= K; i++)
    for (int j = 1; j <= K; j++) {
      V[i][j] = 0;
      for (int k = 1; k <= N; k++) {
        V[i][j] = V[i][j] + d[k][i] * d[k][j] / N;
      }
    }

//  for (int i = 1; i <= K; i++)
//    for (int j = 1; j <= K; j++) {
//      for (int k = 1; k <= N; k++) {
//        if (k == 1)
//          V[i][j] = 0;
//        V[i][j] = V[i][j] + d[k][i] * d[k][j] / N;
//      }
//    }

#pragma endscop
}

//void dsa_kernel(data_t d[N][K], data_t V[K][K]) {
//  data_t d1_ext[K][K][N];
//  data_t d2_ext[K][K][N];
//  data_t V_ext[K][K][N];
//  
//  for (int i = 0; i < K; i++)
//    for (int j = 0; j < K; j++) {
//      for (int n = 0; n < N; n++) {
//        if (j == 0) {
//          d1_ext[i][j][n] = d[n][i];
//        } else {
//          d1_ext[i][j][n] = d1_ext[i][j - 1][n];
//        }
//
//        if (i == 0) {
//          d2_ext[i][j][n] = d[n][j];
//        } else {
//          d2_ext[i][j][n] = d2_ext[i - 1][j][n];
//        }
//
//        if (n == 0) {
//          V_ext[i][j][n] = d1_ext[i][j][n] * d2_ext[i][j][n];
//        } else {
//          V_ext[i][j][n] = V_ext[i][j][n - 1] + d1_ext[i][j][n] * d2_ext[i][j][n];
//        }
//      }
//      V[i][j] = V_ext[i][j][N - 1] / N;
//    }
//}
