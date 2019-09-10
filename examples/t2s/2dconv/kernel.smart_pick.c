#define S1(r,s,i,j) Z_ext[r][s][1][1] = (X[r + 1][s + 1] * W[1][1]);
#define S2(r,s,i,j) Z_ext[r][s][i][1] = (Z_ext[r][s][i - 1][3] + (X[r + i][s + 1] * W[i][1]));
#define S3(r,s,i,j) Z_ext[r][s][i][j] = (Z_ext[r][s][i][j - 1] + (X[r + i][s + j] * W[i][j]));
#define S4(r,s,i,j) Z_ext[r][s][1][j] = (Z_ext[r][s][1][j - 1] + (X[r + 1][s + j] * W[1][j]));
#define S5(r,s,i,j) Z[r][s] = Z_ext[r][s][3][3];

int t1, t2, t3, t4, t5, t6;

/* Start of CLooG code */
for (t1=1;t1<=8;t1++) {
  for (t2=3;t2<=17;t2++) {
    if (t2 <= 10) {
      S1((t2-2),t1,1,1);
    }
    if ((t2 >= 4) && (t2 <= 12)) {
      for (t4=max(2,t2-9);t4<=min(3,t2-2);t4++) {
        S4((t2-t4-1),t1,1,t4);
      }
    }
    for (t3=max(2,ceild(t2-11,2));t3<=min(3,floord(t2-3,2));t3++) {
      if (t2 <= 2*t3+10) {
        S2((t2-2*t3-2),t1,t3,1);
      }
      for (t4=max(t3+2,t2-t3-8);t4<=min(t3+3,t2-t3-1);t4++) {
        S3((t2-t3-t4),t1,t3,(-t3+t4));
      }
    }
    if (t2 >= 10) {
      S5((t2-9),t1,3,3);
    }
  }
}
/* End of CLooG code */
