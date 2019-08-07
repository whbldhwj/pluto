#define S1(r,s,i,j) X_ext[r][0][i][j] = X[r + i][j];
#define S2(r,s,i,j) X_ext[r][s][i][2] = X[r + i][s + 2];
#define S3(r,s,i,j) X_ext[r][s][i][j] = X_ext[r][s - 1][i][j + 1];
#define S4(r,s,i,j) W_ext[0][0][i][j] = W[i][j];
#define S5(r,s,i,j) W_ext[0][s][i][j] = W_ext[0][s - 1][i][j];
#define S6(r,s,i,j) W_ext[r][s][i][j] = W_ext[r - 1][s][i][j];
#define S7(r,s,i,j) Z_ext[r][s][0][0] = (X_ext[r][s][0][0] * W_ext[r][s][0][0]);
#define S8(r,s,i,j) Z_ext[r][s][i][0] = (Z_ext[r][s][i - 1][2] + (X_ext[r][s][i][0] * W_ext[r][s][i][0]));
#define S9(r,s,i,j) Z_ext[r][s][i][j] = (Z_ext[r][s][i][j - 1] + (X_ext[r][s][i][j] * W_ext[r][s][i][j]));

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
for (t1=0;t1<=31;t1++) {
  for (t2=0;t2<=31;t2++) {
    for (t3=0;t3<=2;t3++) {
      for (t4=t2+t3;t4<=t2+t3+2;t4++) {
        if ((t2 == -t3+t4-1) && (t3 >= 1)) {
          S8(t1,t2,t3,0);
        }
        if (t2 >= max(1,-t3+t4-1)) {
          S3(t1,t2,t3,(-t2-t3+t4));
        }
        if ((t2 == -t3+t4-2) && (t2 >= 1)) {
          S2(t1,t2,t3,2);
        }
        if (t2 == 0) {
          S1(t1,0,t3,(-t3+t4));
        }
        if (t1 >= 1) {
          S6(t1,t2,t3,(-t2-t3+t4));
        }
        if ((t1 == 0) && (t2 >= 1)) {
          S5(0,t2,t3,(-t2-t3+t4));
        }
        if ((t1 == 0) && (t2 == 0)) {
          S4(0,0,t3,(-t3+t4));
        }
        if (t2 <= -t3+t4-1) {
          S9(t1,t2,t3,(-t2-t3+t4));
        }
        if ((t2 == t4) && (t3 == 0)) {
          S7(t1,t2,0,0);
        }
      }
    }
  }
}
/* End of CLooG code */
