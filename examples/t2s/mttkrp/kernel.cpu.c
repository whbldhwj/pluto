#define S1(i,j,k,l) D[i][j] = 0;
#define S2(i,j,k,l) D[i][j] = (D[i][j] + ((A[i][k][l] * B[k][j]) * C[l][j]));

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
for (t1=0;t1<=7;t1++) {
  for (t2=0;t2<=56;t2++) {
    for (t3=0;t3<=7;t3++) {
      if (t2 == 0) {
        S1(t3,t1,0,0);
      }
      for (t4=max(0,ceild(t2-7,7));t4<=min(7,floord(t2,7));t4++) {
        S2(t3,t1,t4,(t2-7*t4));
      }
    }
  }
}
/* End of CLooG code */
