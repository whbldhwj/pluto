#define S1(oT4,oT5,oT6,r,s,i,j) Z[r - 1][s - 1] = 0;
#define S2(oT4,oT5,oT6,r,s,i,j) Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];

#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a>b)?a:b)

/*
 * This code implements the 2D convolution (3x3 filter window), which performs:
 * Z(r,s) += X(r+i-1,s+j-1) * W(i,j)
 * Input: X[R+2][S+2], W[3][3]
 * Output: Z[R][S]
 */

#include "kernel.h"

int main(){
  // declarations
  data_t X[R + 2][S + 2];
  data_t W[3][3];
  data_t Z[R][S];
  data_t Z_dsa[R][S];

  // data initialization
  for (int r = 0; r < R + 2; r++)
    for (int s = 0; s < S + 2; s++) {
      X[r][s] = (float)rand() / RAND_MAX;
    }

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
      W[i][j] = (float)rand() / RAND_MAX;
    }

//  for (int r = 0; r < R; r++)
//    for (int s = 0; s < S; s++) {
//      Z[r][s] = 0;
//    }

  // computation
//#pragma scop
//  for (int r = 1; r < R + 1; r++)
//    for (int s = 1 ; s < S + 1; s++) {
//      Z[r - 1][s - 1] = 0;
//      for (int i = 0; i < 3; i++)
//        for (int j = 0; j < 3; j++) {
//          Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];
//        }
//    }
//#pragma endscop  

  int t1, t2, t3, t4, t5, t6, t7, t8;
  
  /* Start of CLooG code */
  for (t1=0;t1<=4;t1++) {
    for (t3=0;t3<=4;t3++) {
      for (t4=max(0,-8*t1+1);t4<=min(7,-8*t1+32);t4++) {
        for (t5=0;t5<=2;t5++) {
          for (t6=max(0,-8*t3+2*t5+1);t6<=min(7,-8*t3+2*t5+34);t6++) {
            for (t7=max(1,8*t3-2*t5+t6-2);t7<=min(32,8*t3-2*t5+t6);t7++) {
              if ((8*t3 == -t6+t7) && (t5 == 0)) {
                S1(t1,0,t3,(8*t3+t6),(8*t1+t4),0,0);
              }
              S2(t1,0,t3,t7,(8*t1+t4),t5,(8*t3-2*t5+t6-t7));
            }
          }
        }
      }
    }
  }
  /* End of CLooG code */

  dsa_kernel(X, W, Z_dsa);

  // comparison
  int err = 0;
  float thres = 0.001;
  for (int r = 0; r < R; r++) 
    for (int s = 0; s < S; s++) {
      if (fabs(Z_dsa[r][s] - Z[r][s]) > thres) {
        err++;
      }
    }

  if (err) {
    printf("Test failed with %d errors!\n", err);
    return -1;
  } else {
    printf("Test passed!\n");
    return 0;
  }
}
