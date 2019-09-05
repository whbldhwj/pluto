/*
 * This code implements the QR decomposition using Givens Rotation
 * QR decompostion decomposes the matrix A into one orthogonal matrix Q and one upper rectangular matrix R.
 * Input: A[M][N] (M >= N)
 * Output: Q[M][M], R[M][N]
 */

#include "kernel.h"

// compute the magnitude of (a, b)
// mag = sqrt(a^2+b^2)
data_t compute_mag(data_t a, data_t b) {
  data_t aa = a * a;
  data_t bb = b * b;
  data_t mag = sqrtf(aa + bb);
  return mag;
}

data_t compute_mm(data_t c, data_t s, data_t *op1, data_t *op2) {
  data_t a = *op1 * c + *op2 * s;
  data_t b = -(*op1) * s + *op2 * c;

  *op1 = a;
  *op2 = b;
}

data_t print_mat(data_t* mat, int row, int col) {
  printf("****\n");
  for (int i = 0; i < row; i++) {
    for (int j = 0; j < col; j++) {
      printf("%f\t", mat[i * col + j]);
    }
    printf("\n");
  }
  printf("****\n");
}

int main(){
  // declarations
  data_t A[M][N];
  data_t Q[M][M];
  data_t R[M][N];
  data_t Q_dsa[M][M];
  data_t R_dsa[M][N];
  
  // data initialization
  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++) {
      A[i][j] = (float)rand() / RAND_MAX;
    }

  // computation
  // 1. initialize Q to identical matrix
  for (int i = 0; i < M; i++)
    for (int j = 0; j < M; j++) {
      if (i == j) 
        Q[i][j] = 1;
      else
        Q[i][j] = 0;
    }

  // 2. initialize R as A
  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++) {
      R[i][j] = A[i][j];
    }

  // 3. apply Given's Rotation
  for (int i = 0; i < N; i++)
    for (int j = M - 1; j > i; j--){
      if (R[j][i] == 0)
        continue;
      else {
        data_t mag = compute_mag(R[j][i], R[j - 1][i]);
        data_t c = R[j - 1][i] / mag; // cos
        data_t s = R[j][i] / mag; // sin
        R[j - 1][i] = mag;
        R[j][i] = 0;
        for (int k = i + 1; k < N; k++) {
          compute_mm(c, s, &R[j - 1][k], &R[j][k]);
        }
        for (int k = 0; k < M; k++) {
          compute_mm(c, s, &Q[k][j - 1], &Q[k][j]);
        }
      }
    }

  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++) {
      R_dsa[i][j] = 0;
    }
  dsa_kernel(A, Q_dsa, R_dsa);

  print_mat((data_t *)A, M, N);
  print_mat((data_t *)Q, M, M);
  print_mat((data_t *)R, M, N);
  print_mat((data_t *)R_dsa, M, N);

  // comparison
  int err = 0;
  float thres = 0.001;
//  for (int i = 0; i < M; i++) 
//    for (int j = 0; j < M; j++) {
//      if (fabs(Q_dsa[i][j] - Q[i][j]) > thres) {
//        err++;
//      }
//    }

  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++) {
      if (fabs(R_dsa[i][j] - R[i][j]) > thres) {
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
