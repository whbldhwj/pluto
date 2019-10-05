#define S1(r,s,i,j) Z[r][s] = 0;
#define S2(r,s,i,j) Z[r][s] = (Z[r][s] + (X[r + i][s + j] * W[i][j]));

int t1, t2, t3, t4;

/* Start of CLooG code */
for (t1=1;t1<=8;t1++) {
  for (t2=1;t2<=3;t2++) {
    for (t3=ceild(3*t2-1,2);t3<=t2+8;t3++) {
      if ((t2 == 1) && (t3 >= 2) && (t3 <= 8)) {
        S2((t3-1),t1,1,1);
        S1(t3,t1,1,1);
      }
      if ((t2 == 1) && (t3 == 1)) {
        S1(1,t1,1,1);
      }
      if ((t2 == 1) && (t3 == 9)) {
        S2(8,t1,1,1);
      }
      if (t2 <= t3-1) {
        for (t4=max(t3+3,t2+t3+1);t4<=t2+t3+3;t4++) {
          S2((-t2+t3),t1,t2,(-t2-t3+t4));
        }
      }
    }
  }
}
/* End of CLooG code */
