#define S1(i,j,k) C[i][j] += A[i][k] * B[k][j];

int t1, t2, t3;

/* Start of CLooG code */
if ((I >= 1) && (J >= 1) && (K >= 1)) {
  for (t1=0;t1<=I-1;t1++) {
    for (t2=0;t2<=J-1;t2++) {
      for (t3=0;t3<=K-1;t3++) {
        S1(t1,t2,t3);
      }
    }
  }
}
/* End of CLooG code */
