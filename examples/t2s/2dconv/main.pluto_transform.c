#define S1(r,s) Z[r - 1][s - 1] = 0;
#define S2(r,s,i,j) Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
if ((R >= 1) && (S >= 1)) {
  for (t1=1;t1<=R;t1++) {
    for (t2=1;t2<=S;t2++) {
      S1(t1,t2);
      for (t3=0;t3<=2;t3++) {
        for (t4=2*t3;t4<=2*t3+2;t4++) {
          S2(t1,t2,t3,(-2*t3+t4));
        }
      }
    }
  }
}
/* End of CLooG code */
