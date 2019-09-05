#define S1(r,s,i,j) Z_ext[r][s][1][1] = (X[r + 1][s + 1] * W[1][1]);
#define S2(r,s,i,j) Z_ext[r][s][i][1] = (Z_ext[r][s][i - 1][3] + (X[r + i][s + 1] * W[i][1]));
#define S3(r,s,i,j) Z_ext[r][s][i][j] = (Z_ext[r][s][i][j - 1] + (X[r + i][s + j] * W[i][j]));
#define S4(r,s,i,j) Z_ext[r][s][1][j] = (Z_ext[r][s][1][j - 1] + (X[r + 1][s + j] * W[1][j]));
#define S5(r,s,i,j) Z[r][s] = Z_ext[r][s][3][3];

int t1, t2, t3, t4, t5, t6;

/* Start of CLooG code */
for (t1=1;t1<=8;t1++) {
  for (t2=3;t2<=17;t2++) {
    for (t3=max(1,t2-9);t3<=min(8,t2-6);t3++) {
      if ((t2+t3)%2 == 0) {
        S2(t1,t3,((t2-t3-2)/2),1);
      }
      t4 = floord(t2-t3+3,2);
      S3(t1,t3,(t2-t3-t4),(-t2+t3+2*t4));
      if (t2 == t3+9) {
        S5(t1,(t2-9),3,3);
      }
    }
    for (t3=max(1,t2-4);t3<=min(8,t2-3);t3++) {
      S4(t1,t3,1,(t2-t3-1));
    }
    if (t2 <= 10) {
      S1(t1,(t2-2),1,1);
    }
  }
}
/* End of CLooG code */
