#define S1(i,j,k) C_ext[i][j][1] = (A[i][1] * B[1][j]);
#define S2(i,j,k) C_ext[i][j][k] = (C_ext[i][j][k - 1] + (A[i][k] * B[k][j]));
#define S3(i,j,k) C[i][j] = C_ext[i][j][8];

int t1, t2, t3, t4;

/* Start of CLooG code */
for (t1=3;t1<=24;t1++) {
  for (t2=max(1,t1-16);t2<=min(8,t1-2);t2++) {
    for (t3=max(1,t1-t2-8);t3<=min(8,t1-t2-2);t3++) {
      S2(t2,t3,(t1-t2-t3));
      if (t1 == t2+t3+8) {
        S3(t2,(t1-t2-8),8);
      }
    }
    if (t1 <= t2+9) {
      S1(t2,(t1-t2-1),1);
    }
  }
}
/* End of CLooG code */
