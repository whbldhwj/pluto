#define S1(i,j) C[i][j] = 0;
#define S2(i,j,k) C[i][j] += (A[i][k] * B[k][j]);

int t1, t2, t3, t4;

/* Start of CLooG code */
for (t1=0;t1<=63;t1++) {
  for (t2=0;t2<=63;t2++) {
    S1(t1,t2);
    for (t3=0;t3<=63;t3++) {
      S2(t1,t2,t3);
    }
  }
}
/* End of CLooG code */
