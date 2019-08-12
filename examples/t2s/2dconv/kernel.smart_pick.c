#define S1(r,s,i,j) Z_ext[r][s][0][0] = (X[r][s] * W[0][0]);
#define S2(r,s,i,j) Z_ext[r][s][i][0] = (Z_ext[r][s][i - 1][2] + (X[r + i][s] * W[i][0]));
#define S3(r,s,i,j) Z_ext[r][s][i][j] = (Z_ext[r][s][i][j - 1] + (X[r + i][s + j] * W[i][j]));
#define S4(r,s,i,j) Z_ext[r][s][0][j] = (Z_ext[r][s][0][j - 1] + (X[r][s + j] * W[0][j]));
#define S5(r,s,i,j) Z[r][s] = Z_ext[r][s][2][2];

int t1, t2, t3, t4, t5, t6;

/* Start of CLooG code */
for (t1=0;t1<=31;t1++) {
  for (t3=0;t3<=31;t3++) {
    S1(t3,t1,0,0);
    for (t4=1;t4<=2;t4++) {
      S4(t3,t1,0,t4);
    }
  }
  for (t2=1;t2<=2;t2++) {
    for (t3=0;t3<=31;t3++) {
      S2(t3,t1,t2,0);
      for (t4=t2+1;t4<=t2+2;t4++) {
        S3(t3,t1,t2,(-t2+t4));
      }
      if (t2 == 2) {
        S5(t3,t1,2,2);
      }
    }
  }
}
/* End of CLooG code */
