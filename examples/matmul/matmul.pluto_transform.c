#define S1(i,j,k) C[i][j] = beta*C[i][j] + alpha*A[i][k] * B[k][j];

int t1, t2, t3;

/* Start of CLooG code */
for (t1=0;t1<=1023;t1++) {
  for (t2=0;t2<=1023;t2++) {
    for (t3=0;t3<=1023;t3++) {
      S1(t1,t2,t3);
    }
  }
}
/* End of CLooG code */
