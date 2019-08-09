#define S1(r,s) Z[r][s] = 0;
#define S2(r,s,i,j) Z[r][s] = Z[r][s] + X[r + i][s + j] * W[i][j];

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
for (t1=0;t1<=31;t1++) {
  for (t2=0;t2<=31;t2++) {
    S1(t2,t1);
    for (t3=0;t3<=2;t3++) {
      for (t4=t2+2*t3;t4<=t2+2*t3+2;t4++) {
        S2(t2,t1,t3,(-t2-2*t3+t4));
      }
    }
  }
}
/* End of CLooG code */
