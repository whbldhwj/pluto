#define S1(r,s,i,j) Z_ext[r][s][1][1] = (X[r + 1][s + 1] * W[1][1]);
#define S2(r,s,i,j) Z_ext[r][s][i][1] = (Z_ext[r][s][i - 1][3] + (X[r + i][s + 1] * W[i][1]));
#define S3(r,s,i,j) Z_ext[r][s][i][j] = (Z_ext[r][s][i][j - 1] + (X[r + i][s + j] * W[i][j]));
#define S4(r,s,i,j) Z_ext[r][s][1][j] = (Z_ext[r][s][1][j - 1] + (X[r + 1][s + j] * W[1][j]));
#define S5(r,s,i,j) Z[r][s] = Z_ext[r][s][3][3];

int t1, t2, t3, t4, t5, t6;

/* Start of CLooG code */
for (t1=1;t1<=8;t1++) {
  for (t2=1;t2<=8;t2++) {
    S1(t1,t2,1,1);
    for (t4=2;t4<=3;t4++) {
      S4(t1,t2,1,t4);
    }
    for (t3=2;t3<=3;t3++) {
      S2(t1,t2,t3,1);
      for (t4=t3+2;t4<=t3+3;t4++) {
        S3(t1,t2,t3,(-t3+t4));
      }
    }
    S5(t1,t2,3,3);
  }
}
/* End of CLooG code */
