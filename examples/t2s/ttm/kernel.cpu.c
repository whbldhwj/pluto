#define S1(i,j,k,l) C[i][j][k] = 0;
#define S2(i,j,k,l) C[i][j][k] = (C[i][j][k] + (A[i][j][l] * B[l][k]));

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
for (t1=1;t1<=8;t1++) {
  for (t2=1;t2<=8;t2++) {
    for (t3=1;t3<=8;t3++) {
      for (t4=1;t4<=8;t4++) {
        if (t2 == 1) {
          S1(t1,t3,t4,1);
        }
        S2(t1,t3,t4,t2);
      }
    }
  }
}
/* End of CLooG code */
