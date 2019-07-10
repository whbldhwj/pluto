#define S1(r,s,i,j) Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];

int t1, t2, t3, t4;

/* Start of CLooG code */
if ((R >= 1) && (S >= 1)) {
  for (t1=1;t1<=S;t1++) {
    for (t2=1;t2<=R;t2++) {
      for (t3=0;t3<=2;t3++) {
        for (t4=t2+2*t3;t4<=t2+2*t3+2;t4++) {
          S1(t2,t1,t3,(-t2-2*t3+t4));
        }
      }
    }
  }
}
/* End of CLooG code */
