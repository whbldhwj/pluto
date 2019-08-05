#define S2(r,s,i,j) Z_ext[r][s][i][j] = Z_ext[r][s][i - 1][2] + X[r + i][s + j] * W[i][j];
#define S1(r,s,i,j) Z_ext[r][s][i][j] = X[r + i][s + j] * W[i][j];
#define S3(r,s,i,j) Z_ext[r][s][i][j] = Z_ext[r][s][i][j - 1] + X[r + i][s + j] * W[i][j];

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
for (t1=0;t1<=31;t1++) {
  for (t2=0;t2<=31;t2++) {
    S2(t2,t1,0,0);
  }
}
/* End of CLooG code */
