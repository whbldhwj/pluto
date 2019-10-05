#include "kernel.h"

/* DSA Form 0 */
// change parameters to constants
// avoid using +=
//void dsa_kernel(data_t A[I + 1][K + 1], data_t B[K + 1][J + 1], data_t C[I + 1][J + 1]) {
//#pragma scop
//  for (int i = 1; i < 8 + 1; i++)
//    for (int j = 1; j < 8 + 1; j++) {
//      for (int k = 1; k < 8 + 1; k++) {
//        if (k == 1)
//          C[i][j] = 0;
//        C[i][j] = C[i][j] + A[i][k] * B[k][j];
//      }
//    }
//#pragma endscop
//}

/* DSA Form 1 */
void dsa_kernel(data_t A[I + 1][K + 1], data_t B[K + 1][J + 1], data_t C[I + 1][J + 1]) {
  data_t C_ext[8 + 1][8 + 1][8 + 1];
#pragma scop
  for (int i = 1; i < 8 + 1; i ++)
    for (int j = 1; j < 8 + 1; j++)
      for (int k = 1; k < 8 + 1; k++) {
        if (k == 1)
          C_ext[i][j][k] = A[i][k] * B[k][j];
        else
          C_ext[i][j][k] = C_ext[i][j][k - 1] + A[i][k] * B[k][j];        
        if (k == 8)
          C[i][j] = C_ext[i][j][k];
      }
#pragma endscop  
  
  for (int i = 1; i < 8 + 1; i++)
    for (int j = 1; j < 8 + 1; j++) {
      C[i][j] = C_ext[i][j][8];
    }
}
