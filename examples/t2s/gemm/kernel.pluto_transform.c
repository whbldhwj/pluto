#define S1(i,j,k) C[i][j] = 0;
#define S2(i,j,k) C[i][j] = (C[i][j] + (A[i][k] * B[k][j]));

int t1, t2, t3, t4;

/* Start of CLooG code */
for (t1=1;t1<=8;t1++) {
  for (t2=1;t2<=8;t2++) {
    S1(t1,t2,1);
    for (t3=1;t3<=8;t3++) {
      S2(t1,t2,t3);
    }
  }
}
/* End of CLooG code */
