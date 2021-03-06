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

//void dsa_kernel(data_t X[R + 3 + 1][S + 3 + 1], data_t W[3 + 1][3 + 1], data_t Z[R + 1][S + 1]) {
//  data_t Z_ext[8 + 1][17 + 1][8 + 1][8 + 1];
//  data_t X_CC1_E[8 + 1][17 + 1][8 + 1][8 + 1];
//  data_t X_CC3_E[8 + 1][17 + 1][8 + 1][8 + 1];
//  data_t X_CC5_E[8 + 1][17 + 1][8 + 1][8 + 1];
//  data_t W_CC0_E[8 + 1][17 + 1][8 + 1][8 + 1];
//  data_t W_CC4_E[8 + 1][17 + 1][8 + 1][8 + 1];
//  data_t W_CC6_E[8 + 1][17 + 1][8 + 1][8 + 1];
//
//  for (int t1 = 1; t1 < 8 + 1; t1++)
//    for (int t2 = 3; t2 < 17 + 1; t2++) 
//      for (int t3 = 1; t3 < 8 + 1; t3++)
//        for (int t4 = 1; t4 < 8 + 1; t4++) {
//          // S4 + S1
//          if ((-t2 + t4 + 2 == 0 && t3 - 1 == 0 && t2 - 4 >= 0 && -t2 + 5 >= 0) || (t4 - 1 == 0 && t3 - 1 == 0 && t2 - 3 == 0)) {
//            W_CC0_E[t1][t2][t3][t4] = W[t3][t4];
//          } else if ((t3 - 1 == 0 && t4 - 2 >= 0 && -t4 + 3 >= 0 && t2 - t4 - 3 >= 0) || (t4 - 1 == 0 && t3 - 1 == 0 && t2 - 4 >= 0)) {
//            W_CC0_E[t1][t2][t3][t4] = W_CC0_E[t1][t2 - 1][t3][t4];
//          } else {
//            W_CC0_E[t1][t2][t3][t4] = W_CC0_E[t1][t2][t3][t4];  
//          }
//          
//          // S4 + S1
//          if ((t3 - 1 == 0 && -t4 + 3 >= 0 && t4 - 2 >= 0) || (t4 - 1 == 0 && t3 - 1 == 0)) {
//            X_CC1_E[t1][t2][t3][t4] = X[t2 - t4][t1 + t4];
//          } else {
//            X_CC1_E[t1][t2][t3][t4] = X_CC1_E[t1][t2][t3][t4];
//          }
//
//          // S2
//          if ((t4 - 4 == 0 && t3 - 2 == 0) || (t4 - 5 == 0 && t3 - 3 == 0 && t2 - 16 == 0)) {
//            X_CC3_E[t1][t2][t3][t4] = X[t2 - t4][t1 - t3 + t4 - 1];
//          } else if ((t4 - 5 == 0 && t3 - 3 == 0 && -t2 + 15 >= 0)) {
//            X_CC3_E[t1][t2][t3][t4] = X_CC3_E[t1][t2 - 1][t3 - 1][t4 - 1];
//          } else {
//            X_CC3_E[t1][t2][t3][t4] = X_CC3_E[t1][t2][t3][t4];
//          }
//
//          // S2
//          if ((-t2 + 2 * t4 - 1 == 0 && -t2 + 2 * t3 + 3 == 0 && t2 - 7 >= 0)) {
//            W_CC4_E[t1][t2][t3][t4] = W[t3][-t3 + t4 - 1];
//          } else if ((-t3 + t4 - 2 == 0 && t3 - 2 >= 0 && t2 - 2 * t3 - 4 >= 0)) {
//            W_CC4_E[t1][t2][t3][t4] = W_CC4_E[t1][t2 - 1][t3][t4];
//          } else {
//            W_CC4_E[t1][t2][t3][t4] = W_CC4_E[t1][t2][t3][t4];
//          }
//
//          // S3
//          if ((t3 - 2 == 0 && t4 - 4 >= 0) || (-t2 + t4 + 11 == 0 && t3 - 3 == 0 && t2 - 16 >= 0)) {
//            X_CC5_E[t1][t2][t3][t4] = X[t2 - t4][t1 - t3 + t4];
//          } else if ((t3 - 3 == 0 && -t2 + t4 + 10 >= 0 && t4 - 5 >= 0)) {
//            X_CC5_E[t1][t2][t3][t4] = X_CC5_E[t1][t2 - 1][t3 - 1][t4 - 1];
//          } else {
//            X_CC5_E[t1][t2][t3][t4] = X_CC5_E[t1][t2][t3][t4];
//          }
//
//          // S3
//          if ((-t2 + t3 + t4 + 1 == 0 && t3 - 2 >= 0 && t2 - 2 * t3 - 3 >= 0)) {
//            W_CC6_E[t1][t2][t3][t4] = W[t3][-t3 + t4];
//          } else if ((-t3 + t4 - 2 >= 0 && t2 - t3 - t4 - 2 >= 0 && t3 - 2 >= 0)) {
//            W_CC6_E[t1][t2][t3][t4] = W_CC6_E[t1][t2 - 1][t3][t4];
//          } else {
//            W_CC6_E[t1][t2][t3][t4] = W_CC6_E[t1][t2][t3][t4];
//          }
//
//          if (t4 - 1 == 0 && t3 - 1 == 0) {
//            Z_ext[t1][t2][t3][t4] = X_CC1_E[t1][t2][t3][t4] * W_CC0_E[t1][t2][t3][t4]; // S1
//            if (t1 == 1 && t2 == 3 && t3 == 1 && t4 == 1)
//              printf("%d %d %d\n", X_CC1_E[t1][t2][t3][t4], W_CC0_E[t1][t2][t3][t4], Z_ext[t1][t2][t3][t4]);
//          } 
//          if (t4 - 4 == 0 && t3 - 2 == 0) {
//            Z_ext[t1][t2][t3][t4] = Z_ext[t1][t2 - 2][t3 - 1][t4 - 1] + X_CC3_E[t1][t2][t3][t4] * W_CC4_E[t1][t2][t3][t4]; // S2 from S4
//            if (t1 == 1 && t2 == 1 + 4 + 1 + 1 && t3 == 2 && t4 == 4) {
//              printf("%d %d %d\n", X_CC3_E[t1][t2][t3][t4], W_CC4_E[t1][t2][t3][t4], Z_ext[t1][t2][t3][t4]);
//            }
//
//          } 
//          if (t4 - 5 == 0 && t3 - 3 == 0) {
//            Z_ext[t1][t2][t3][t4] = Z_ext[t1][t2 - 1][t3 - 1][t4] + X_CC3_E[t1][t2][t3][t4] * W_CC4_E[t1][t2][t3][t4]; // S2 from S3
//          } 
//          if (-t3 + t4 - 2 == 0 && t3 - 2 >= 0) {
//            Z_ext[t1][t2][t3][t4] = Z_ext[t1][t2][t3][t4] + X_CC5_E[t1][t2][t3][t4] * W_CC6_E[t1][t2][t3][t4]; // S3 from S2
//            if (t1 == 1 && t2 == 1 + 4 + 2 && t3 == 2 && t4 == 4) {
//              printf("%d %d %d\n", X_CC5_E[t1][t2][t3][t4], W_CC6_E[t1][t2][t3][t4], Z_ext[t1][t2][t3][t4]);
//            }
//
//          } 
//          if (-t3 + t4 - 3 == 0 && t3 - 2 >= 0) {
//            Z_ext[t1][t2][t3][t4] = Z_ext[t1][t2 - 1][t3][t4 - 1] + X_CC5_E[t1][t2][t3][t4] * W_CC6_E[t1][t2][t3][t4]; // S3 from S3
//          } 
//          if (t3 - 1 == 0 && -t4 + 3 >= 0 && t4 - 2 >= 0) {
//            Z_ext[t1][t2][t3][t4] = Z_ext[t1][t2 - 1][t3][t4 - 1] + X_CC1_E[t1][t2][t3][t4] * W_CC0_E[t1][t2][t3][t4]; // S4
//            if (t1 == 1 && t2 == 4 && t3 == 1 && t4 == 2) {
//              printf("%d %d %d\n", X_CC1_E[t1][t2][t3][t4], W_CC0_E[t1][t2][t3][t4], Z_ext[t1][t2][t3][t4]);
//            }
//            if (t1 == 1 && t2 == 5 && t3 == 1 && t4 == 3) {
//              printf("%d %d %d\n", X_CC1_E[t1][t2][t3][t4], W_CC0_E[t1][t2][t3][t4], Z_ext[t1][t2][t3][t4]);
//            }
//          } 
////          else {
////            Z_ext[t1][t2][t3][t4] = Z_ext[t1][t2][t3][t4];
////          }
//
//
//          if (t4 - 6 == 0 && t3 - 3 == 0) {
//            if (t2 - t3 - t4 >= 0 && t2 - t3 - t4 <= 8)
//              Z[t2 - t3 - t4][t1] = Z_ext[t1][t2][t3][t4]; // S5
//          }
//
//        }
//
//}

//void dsa_kernel(data_t X[R + 3 + 1][S + 3 + 1], data_t W[3 + 1][3 + 1], data_t Z[R + 1][S + 1]) {
//  data_t Z_ext[R + 1][S + 1][3 + 1][3 + 1];
//
//#define ceild(n,d)  ceil(((double)(n))/((double)(d)))
//#define floord(n,d) floor(((double)(n))/((double)(d)))
//#define max(x,y)    ((x) > (y)? (x) : (y))
//#define min(x,y)    ((x) < (y)? (x) : (y))
//#define S1(r,s,i,j) Z_ext[r][s][1][1] = (X[r + 1][s + 1] * W[1][1]);
//#define S2(r,s,i,j) Z_ext[r][s][i][1] = (Z_ext[r][s][i - 1][3] + (X[r + i][s + 1] * W[i][1]));
//#define S3(r,s,i,j) Z_ext[r][s][i][j] = (Z_ext[r][s][i][j - 1] + (X[r + i][s + j] * W[i][j]));
//#define S4(r,s,i,j) Z_ext[r][s][1][j] = (Z_ext[r][s][1][j - 1] + (X[r + 1][s + j] * W[1][j]));
//#define S5(r,s,i,j) Z[r][s] = Z_ext[r][s][3][3];
//
//int t1, t2, t3, t4, t5, t6;
//
///* Start of CLooG code */
//for (t1=1;t1<=8;t1++) {
//  for (t2=3;t2<=17;t2++) {
//    if (t2 <= 10) {
//      S1(t1,(t2-2),1,1);
//    }
//    if ((t2 >= 4) && (t2 <= 12)) {
//      for (t4=max(2,t2-9);t4<=min(3,t2-2);t4++) {
//        S4(t1,(t2-t4-1),1,t4);
//      }
//    }
//    for (t3=max(2,ceild(t2-11,2));t3<=min(3,floord(t2-3,2));t3++) {
//      if (t2 <= 2*t3+10) {
//        S2(t1,(t2-2*t3-2),t3,1);
//      }
//      for (t4=max(t3+2,t2-t3-8);t4<=min(t3+3,t2-t3-1);t4++) {
//        S3(t1,(t2-t3-t4),t3,(-t3+t4));
//      }
//    }
//    if (t2 >= 10) {
//      S5(t1,(t2-9),3,3);
//    }
//  }
//}
///* End of CLooG code */
//
//#pragma endscop  
//
//  for (int r = 1; r < 8 + 1; r++)
//    for (int s = 1; s < 8 + 1; s++) {
//      Z[r][s] = Z_ext[r][s][3][3];
//    }
//}


/* DSA From 2 */
//void dsa_kernel(data_t X[R + 2][S + 2], data_t W[3][3], data_t Z[R][S]) {
//  data_t X_ext[R][S][3][3];
//  data_t W_ext[R][S][3][3];
//  data_t Z_ext[R][S][3][3];
//
//#pragma scop
//  for (int r = 0; r < R; r++)
//    for (int s = 0; s < S; s++) {
//      for (int i = 0; i < 3; i++) {
//        for (int j = 0; j < 3; j++) {
//          // reuse at (1, -1) / (s, j) direction
//          if (s == 0) {
//            X_ext[r][s][i][j] = X[r + i][s + j];
//          } else if (j == 3 - 1 && s > 0) {
//            X_ext[r][s][i][j] = X[r + i][s + j];
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
//          if (i == 0 && j == 0) {
//            Z_ext[r][s][i][j] = X_ext[r][s][i][j] * W_ext[r][s][i][j];
//          } else {
//            if (i > 0 && j == 0) {
//              Z_ext[r][s][i][j] = Z_ext[r][s][i - 1][3 - 1] + X_ext[r][s][i][j] * W_ext[r][s][i][j];
//            } else {
//              Z_ext[r][s][i][j] = Z_ext[r][s][i][j - 1] + X_ext[r][s][i][j] * W_ext[r][s][i][j];
//            }
//          }
//        }
//      }
//    }
//#pragma endscop
//
//  for (int r = 0; r < R; r++)
//    for (int s = 0; s < S; s++) {
//      Z[r][s] = Z_ext[r][s][3 - 1][3 - 1];
//    }
//}
