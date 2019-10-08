#define S1(i,j,k) D[i][j][k] = 0;
#define S2(i,j,k,l,m) D[i][j][k] = (D[i][j][k] + ((A[i][l][m] * B[l][j]) * C[m][k]));

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
for (t1=1;t1<=8;t1++) {
  for (t2=1;t2<=8;t2++) {
    for (t3=1;t3<=8;t3++) {
      S1(t1,t2,t3);
      for (t4=1;t4<=8;t4++) {
        for (t5=7*t4+1;t5<=7*t4+8;t5++) {
          S2(t1,t2,t3,t4,(-7*t4+t5));
        }
      }
    }
  }
}
/* End of CLooG code */
