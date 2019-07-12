#define S1(r,s,i,j) Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];

int t1, t2, t3, t4;

/* Start of CLooG code */
if ((R >= 1) && (S >= 1)) {
  for (t1=1;t1<=S;t1++) {
    for (t2=0;t2<=2;t2++) {
      for (t3=1;t3<=R;t3++) {
        for (t4=2*t2+t3;t4<=2*t2+t3+2;t4++) {
          S1(t3,t1,t2,(-2*t2-t3+t4));
        }
      }
    }
  }
}
/* End of CLooG code */
