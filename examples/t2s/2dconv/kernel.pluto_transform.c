#define S1(r,s,i,j) Z[r][s] = (Z[r][s] + (X[r + i][s + j] * W[i][j]));

int t1, t2, t3, t4;

/* Start of CLooG code */
for (t1=1;t1<=8;t1++) {
  for (t2=1;t2<=3;t2++) {
    for (t3=t2+1;t3<=t2+8;t3++) {
      for (t4=t2+t3+1;t4<=t2+t3+3;t4++) {
        S1((-t2+t3),t1,t2,(-t2-t3+t4));
      }
    }
  }
}
/* End of CLooG code */
