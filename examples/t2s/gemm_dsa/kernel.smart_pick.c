#define S1(i,j,k) C_ext[i][j][k] = 0;
#define S2(i,j,k) C_ext[i][j][k + 1] = C_ext[i][j][k] + A[i][k] * B[k][j];

int t1, t2, t3, t4;

/* Start of CLooG code */
if ((I >= 1) && (J >= 1) && (K >= 1)) {
  for (t1=0;t1<=I-1;t1++) {
    for (t2=0;t2<=J-1;t2++) {
      S1(t1,t2,0);
      for (t3=0;t3<=K-1;t3++) {
        S2(t1,t2,t3);
      }
    }
  }
}
/* End of CLooG code */
