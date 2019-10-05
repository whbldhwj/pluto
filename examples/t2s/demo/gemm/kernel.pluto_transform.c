#define S1(i,j,k) C_ext[i][j][1] = (A[i][1] * B[1][j]);
#define S2(i,j,k) C_ext[i][j][k] = (C_ext[i][j][k - 1] + (A[i][k] * B[k][j]));
#define S3(i,j,k) C[i][j] = C_ext[i][j][8];

int t1, t2, t3, t4;

/* Start of CLooG code */
for (t1=1;t1<=8;t1++) {
  for (t2=1;t2<=8;t2++) {
    S1(t1,t2,1);
    for (t3=2;t3<=8;t3++) {
      S2(t1,t2,t3);
    }
    S3(t1,t2,8);
  }
}
/* End of CLooG code */
