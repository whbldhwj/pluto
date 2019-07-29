#define S1(i,j,k) C_ext[i][j][k] = 0;
#define S2(i,j,k) C_ext[i][j][k + 1] = C_ext[i][j][k] + A[i][k] * B[k][j];
#define S3(i,j,k) C[i][j] = C_ext[i][j][k + 1];

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
for (t1=0;t1<=63;t1++) {
  for (t2=0;t2<=63;t2++) {
    S1(t1,t2,0);
    for (t3=0;t3<=63;t3++) {
      S2(t1,t2,t3);
    }
    S3(t1,t2,63);
  }
}
/* End of CLooG code */
