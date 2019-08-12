#define S1(i,j,k) C_ext[i][j][0] = (A[i][0] * B[0][j]);
#define S2(i,j,k) C_ext[i][j][k] = (C_ext[i][j][k - 1] + (A[i][k] * B[k][j]));
#define S3(i,j,k) C[i][j] = C_ext[i][j][63];

int t1, t2, t3, t4;

/* Start of CLooG code */
for (t1=0;t1<=63;t1++) {
  for (t2=0;t2<=63;t2++) {
    S1(t1,t2,0);
    for (t3=1;t3<=63;t3++) {
      S2(t1,t2,t3);
    }
    S3(t1,t2,63);
  }
}
/* End of CLooG code */
