#define S1(r,s,i,j) Z_ext[r][s][0][0] = (X[r][s] * W[0][0]);
#define S2(r,s,i,j) Z_ext[r][s][i][0] = (Z_ext[r][s][i - 1][2] + (X[r + i][s] * W[i][0]));
#define S3(r,s,i,j) Z_ext[r][s][i][j] = (Z_ext[r][s][i][j - 1] + (X[r + i][s + j] * W[i][j]));
#define S4(r,s,i,j) Z_ext[r][s][0][j] = (Z_ext[r][s][0][j - 1] + (X[r][s + j] * W[0][j]));

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
for (t1=0;t1<=31;t1++) {
  for (t2=0;t2<=31;t2++) {
    S1(t1,t2,0,0);
    for (t4=1;t4<=2;t4++) {
      S4(t1,t2,0,t4);
    }
    for (t3=1;t3<=2;t3++) {
      S2(t1,t2,t3,0);
      for (t4=t3+1;t4<=t3+2;t4++) {
        S3(t1,t2,t3,(-t3+t4));
      }
    }
  }
}
/* End of CLooG code */
