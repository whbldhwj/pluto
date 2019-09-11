#define S1(i,j,k) C[i][j] = 0;
#define S2(i,j,k) C[i][j] = (C[i][j] + (A[i][k] * B[k][j]));

int t1, t2, t3, t4;

/* Start of CLooG code */
for (t1=3;t1<=24;t1++) {
  for (t2=max(1,t1-16);t2<=min(8,t1-2);t2++) {
    for (t3=max(1,t1-t2-8);t3<=min(8,t1-t2-1);t3++) {
      if (t1 == t2+t3+1) {
        S1(t2,(t1-t2-1),1);
      }
      S2(t2,t3,(t1-t2-t3));
    }
  }
}
/* End of CLooG code */
